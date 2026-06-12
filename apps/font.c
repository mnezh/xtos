#include "xtos/app.h"
#include "xtos/display.h"
#include "xtos/event.h"
#include "xtos/ui/canvas.h"
#include "xtos/ui/font.h"
#include "xtos/ui/form.h"
#include "runtime/log.h"

#define FONT_COUNT 3
#define PREVIEW_LEFT 95
#define PREVIEW_TOP 6
#define PREVIEW_RIGHT 317
#define PREVIEW_BOTTOM 42
#define GLYPH_LEFT 9
#define GLYPH_TOP 62
#define GLYPH_RIGHT 317
#define GLYPH_BOTTOM 188

static Form MainForm;
static List FontList;
static Label FontListLabel;
static View PreviewView;
static View GlyphView;

static const Font *fonts[FONT_COUNT];

static const char *font_names[FONT_COUNT];

static void draw_printable_glyphs(View *view, const Font *font)
{
    char text[4];
    u16 x;
    u16 y;
    u16 glyph;
    u16 codepoint;
    u8 advance;

    x = view->left;
    y = view->top;

    for (glyph = 0; glyph < FontGlyphCount(font); ++glyph) {
        codepoint = FontCodepointAt(font, glyph);

        if (codepoint < 32 || codepoint == 127) {
            continue;
        }

        advance = FontGlyphWidthAt(font, glyph);

        if (DisplayCurrentMode() == DISPLAY_MODE_HIGH) {
            advance = (u8)((advance + 1) >> 1);
        }

        if ((u16)(x + advance) >= view->right) {
            x = view->left;
            y = (u16)(y + FontHeight(font) + 2);
        }

        if ((u16)(y + FontHeight(font)) >= view->bottom) {
            return;
        }

        if (codepoint < 0x80) {
            text[0] = (char)codepoint;
            text[1] = 0;
        } else if (codepoint < 0x800) {
            text[0] = (char)(0xc0 | (codepoint >> 6));
            text[1] = (char)(0x80 | (codepoint & 0x3f));
            text[2] = 0;
        } else {
            text[0] = (char)(0xe0 | (codepoint >> 12));
            text[1] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
            text[2] = (char)(0x80 | (codepoint & 0x3f));
            text[3] = 0;
        }

        CanvasText(x, y, font, CANVAS_PRIMARY_FOREGROUND, text);
        x = (u16)(x + advance + 2);
    }
}

static void draw_preview_view(View *view, const Rect *dirty, void *data)
{
    const Font *font;
    u16 y;

    (void)dirty;
    (void)data;

    font = fonts[ListSelected(&FontList)];
    y = view->top;

    CanvasText(view->left, y, font,
               CANVAS_PRIMARY_FOREGROUND,
               "The quick brown fox");
    y = (u16)(y + FontHeight(font) + 2);
    CanvasText(view->left, y, font,
               CANVAS_PRIMARY_FOREGROUND,
               "Příliš žluťoučký kůň úpěl ďábelské ódy");
    y = (u16)(y + FontHeight(font) + 2);
    CanvasText(view->left, y, font,
               CANVAS_PRIMARY_FOREGROUND,
               "Эй, жлоб! Где туз? Прячь юных съёмщиц в шкаф.");
}

static void draw_glyph_view(View *view, const Rect *dirty, void *data)
{
    (void)dirty;
    (void)data;

    draw_printable_glyphs(view, fonts[ListSelected(&FontList)]);
}

static void FontViewerInit(void)
{
    fonts[0] = FontGet(FONT_SMALL);
    fonts[1] = FontGet(FONT_SYSTEM);
    fonts[2] = FontGet(FONT_LARGE);
    font_names[0] = FontName(fonts[0]);
    font_names[1] = FontName(fonts[1]);
    font_names[2] = FontName(fonts[2]);
    XTOS_LOG_PREFIX("[FONTAPP]", "names ready");

#ifdef XTOS_DEBUG
    FontDebugSnapshotSelect(fonts[0]);
#endif

    XTOS_LOG_PREFIX("[FONTAPP]", "form init begin");
    FormInit(&MainForm, "Font Viewer", FontGet(FONT_SYSTEM));
    LabelInit(&FontListLabel, 9, 6, FontGet(FONT_SMALL), "Select font:",
              CANVAS_PRIMARY_FOREGROUND);
    ListInit(&FontList, 7, 14, 84, FontGet(FONT_SMALL), font_names,
             FONT_COUNT);
    XTOS_LOG_PREFIX("[FONTAPP]", "form init done");
    ViewInit(&PreviewView, PREVIEW_LEFT, PREVIEW_TOP,
             PREVIEW_RIGHT, PREVIEW_BOTTOM, draw_preview_view, 0);
    ViewInit(&GlyphView, GLYPH_LEFT, GLYPH_TOP,
             GLYPH_RIGHT, GLYPH_BOTTOM, draw_glyph_view, 0);

    FormAddList(&MainForm, &FontList);
    FormAddLabel(&MainForm, &FontListLabel);
    FormAddView(&MainForm, &PreviewView);
    FormAddView(&MainForm, &GlyphView);
    ViewInvalidate(&PreviewView);
    ViewInvalidate(&GlyphView);
}

static void FontViewerHandleEvent(Event *event)
{
    u8 before;
    int action;

    if (event->type == EVENT_KEYDOWN && event->key == XTOS_KEY_ALT_Q) {
        AppQuit();
        return;
    }

    if (event->type == EVENT_KEYDOWN &&
        (event->key == 'm' || event->key == 'M')) {
        DisplaySetMode(DisplayCurrentMode() == DISPLAY_MODE_LOW ?
                       DISPLAY_MODE_HIGH : DISPLAY_MODE_LOW);
        return;
    }

    before = ListSelected(&FontList);
    action = FormHandleEvent(&MainForm, event);

    if (action == FORM_ACTION_CLOSE) {
        AppQuit();
        return;
    }

    if (before != ListSelected(&FontList)) {
        ViewInvalidate(&PreviewView);
        ViewInvalidate(&GlyphView);
    }
}

static void FontViewerDraw(void)
{
    FormDraw(&MainForm);
}

static void FontViewerShutdown(void)
{
    DisplayShutdown();
}

int main(void)
{
    static Application FontViewerApp;

    FontViewerApp.Init = FontViewerInit;
    FontViewerApp.HandleEvent = FontViewerHandleEvent;
    FontViewerApp.Draw = FontViewerDraw;
    FontViewerApp.Shutdown = FontViewerShutdown;
    FontViewerApp.first_draw_screenshot_path = "font.cga";

    return AppRun(&FontViewerApp);
}
