# XTOS API Specification (v0.1)

---

## 1. Event API

```c
int GetEvent(Event *e);
void PostEvent(const Event *e);
void ExitApp(void);
```

### Event structure

```c
typedef struct {
    int type;
    int x;
    int y;
    int key;
    int data;
} Event;
```

---

## 2. File API

```c
File FileOpen(const char *path, const char *mode);
int  FileRead(File f, void *buf, int size);
int  FileWrite(File f, const void *buf, int size);
void FileClose(File f);

int FileList(const char *path, FileEntry *out, int max);
```

### FileEntry

```c
typedef struct {
    char name[32];
    int size;
    int flags;
} FileEntry;
```

---

## 3. Execution API

```c
int AppRun(const char *path, const char *args);
void AppQuit(void);
```

---

## 4. UI API

---

### Forms

```c
Form* FormOpen(int id);
void  FormClose(Form *f);

int FormHandleEvent(Form *f, Event *e);
void FormDraw(Form *f);
```

---

### Canvas

```c
void CanvasBegin(void);
void CanvasEnd(void);

void DrawPixel(int x, int y);
void DrawLine(int x1, int y1, int x2, int y2);
void DrawRect(int x, int y, int w, int h);
void DrawText(int x, int y, const char *s);
```

---

### Document Views

```c
DocView* DocOpen(int type);
void DocClose(DocView *v);

void DocRender(DocView *v, int x, int y, int w, int h);
void DocHandleEvent(DocView *v, Event *e);
```

---

## Design Rules

- No object-oriented UI system
- No window-per-control architecture
- No hidden control flow
- All UI driven by explicit event loop
