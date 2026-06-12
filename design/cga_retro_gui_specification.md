# Retro GUI Specifications: Mapping Macintosh UI to CGA Display

A comprehensive engineering guide and design specification for re-implementing a classic 1:1 square-pixel desktop user interface on a vintage 320×200, 4-color CGA canvas with tall rectangular pixels.

---

## 1. Core Mathematical Scaling Framework

Direct bilinear or fractional image scaling of a bitmap-based interface results in unreadable typography and broken single-pixel borders. To preserve the iconic aesthetic of the original Macintosh while working inside the hardware realities of IBM's Color Graphics Adapter, you must implement an **asymmetric, aspect-corrected coordinate mapping engine**.

### Screen Dimension Disparity
* **Original Macintosh (1984):** $512 \times 342$ pixels (Physical 4:3 Aspect Ratio, Square Pixels, $1.0$ PAR)
* **CGA Mode 4 (320×200):** $320 \times 200$ pixels (Physical 4:3 Aspect Ratio, Tall Rectangular Pixels, $1.2$ PAR)

### The Aspect-Corrected Scaling Rule
To ensure that visual shapes (such as circular buttons, square icons, and window drop-shadows) do not appear vertically stretched on a physical CGA CRT monitor, the coordinate transform must explicitly cancel out the **1:1.2 Pixel Aspect Ratio (PAR)**.

The optimal uniform visual scale factor is **$0.6\times$ Horizontal** and **$0.5\times$ Vertical**.

$$\begin{aligned}
X_{\text{CGA}} &= \text{round}(X_{\text{Mac}} \times 0.6) \\
Y_{\text{CGA}} &= \text{round}(Y_{\text{Mac}} \times 0.5)
\end{aligned}$$

### Layout Letterboxing & Centering
Applying this scale directly transforms the Mac's full $512 \times 342$ workspace into a $308 \times 171$ active viewport. To keep the interface centered natively on the CGA CRT, inject a dead-space border wrapper into your rendering pipeline:

* **Left / Right Margin Padding:** 6 pixels (`X_offset = 6`)
* **Top / Bottom Margin Padding:** 14 pixels (`Y_offset = 14`)
* **Total Usable Framework Canvas:** $308 \times 172$ pixels.

---

## 2. Typography & Low-Resolution Font Metrics

Text legibility dictates the entire layout structure. Standard 8×8 system fonts take up too much vertical space, whereas unoptimized small fonts look distorted due to CGA’s tall pixels.

### System Body Font: 5×7 Monospace
* **Glyph Bounding Box:** 5 pixels wide by 7 pixels high.
* **Layout Grid Cell:** 6 pixels wide by 8 pixels high (provides mandatory 1-pixel horizontal tracking and 1-pixel vertical line leading).
* **Maximum Character Capacity:**
  * **Characters Per Line (CPL):** 53 characters maximum across the active 320-pixel horizontal canvas.
  * **Lines Per Screen (LPS):** 25 rows of text max vertically.

### UI Title Font: 5×7 Bold (Double-Strike Variant)
Because the classic `misc-fixed` 5×7 bitmap has no native bold weight, your software text renderer must perform an **inline overstrike operation**:
1. Blit the standard 5×7 glyph at coordinates $(X, Y)$.
2. Blit the exact same glyph mask a second time shifted at $(X + 1, Y)$.
3. **Layout Adjustment:** Force the text-engine layout to advance the character width by **7 pixels** horizontally instead of the normal 6 pixels to prevent adjacent characters from overlapping.

```
Standard 'A' (5x7)             Bold Overstrike 'A' (6x7)
  . . ■ . .                      . . ■ ■ . .
  . ■ . ■ .                      . ■ ■ . ■ .
  . ■ ■ ■ .    ======>           . ■ ■ ■ ■ .
  ■ . . . ■                      ■ ■ . . . ■
  ■ . . . ■                      ■ ■ . . . ■
```

---

## 3. Pixel-Perfect UI Element Dimensions

The following dimensions are exact targets optimized to minimize pixel jitter and preserve the human-factors aspect of cursor tracking at low resolutions.

### The Global Menu Bar
The top menu bar drops cleanly to an exact integer half-height:
* **Total Height:** 10 pixels.
  * *Top Margin Padding:* 1 pixel.
  * *Font Text Row:* 7 pixels (Standard 5×7 font).
  * *Bottom Margin Padding:* 1 pixel.
  * *Seam Line (Solid Shadow Drop):* 1 pixel (Black).
* **Menu Item Horizontal Spacing:** 12 pixels minimum padding between text string categories.

### Window Framework & Chrome
* **Window Title Bar Height:** 11 pixels total (Includes 1-pixel upper window border, 7-pixel text height, 2-pixel title accent lines, and 1-pixel lower title bar separator).
* **Structural Borders:** Exactly 1 pixel thick.
* **Window Drop-Shadow:** Exactly 1 pixel thick, offset diagonally down and to the right by exactly 1 pixel (`+1X, +1Y`). 
  * *Design Constraint:* Avoid 2-pixel thick shadows; they consume crucial interactive layout areas.
* **Close Box ("Go-Away" Region):** 7×7 pixels square, centered vertically within the title bar, positioned 4 pixels inward from the left structural window boundary.
* **Window Resize Box (Grip):** 9×8 pixels rectangular, placed in the absolute bottom-right vertex of active document windows.

### Interactive Controls & Buttons
* **Standard Desktop Button Height:** 12 pixels high total.
* **Button Width:** Contextual based on text string length, minimum 30 pixels.
* **Corner Radius Modulo:** 2-pixel corner mitigation. Truncating a single corner pixel diagonally provides the clean illusion of an Apple rounded rectangle at 320×200.
* **Internal Text Alignment Padding:** 2 pixels of solid padding to the left and right of the text bounding string.

### Desktop Icon Architecture
* **Large Application/System Icons:** 16×16 pixels square (perfectly down-sampled from the original Mac 32×32 assets to fit an optimized 2-byte grid array).
* **Small Document/Toolbar Icons:** 8×8 pixels square.
* **Icon Label Spacing:** Text fields must be drawn precisely 2 pixels below the bottom boundary of the icon asset graphic, limited strictly to 1 line of text with an ellipses truncation indicator for strings exceeding 10 characters.

---

## 4. Engineering Metric Translation Mapping

| UI Element Description | Original Mac Measure (px) | CGA Target Measure (px) | Engineering Implementation Rules & Notes |
| :--- | :---: | :---: | :--- |
| **Menu Bar Total Height** | 20 | **10** | Exact 50% scale. Fits 5×7 font with single pixel padding. |
| **Application Icon Frame** | 32 × 32 | **16 × 16** | Clean power-of-two reduction. Fits into 32 bytes of VRAM. |
| **Document Icon Frame** | 16 × 16 | **8 × 8** | Scaled for toolbars and lists. Fits into 8 bytes of VRAM. |
| **Scroll Bar Column Width**| 16 | **10** | Adjusted from mathematical scale (9.6) to keep thumb track centered. |
| **Alert/Dialog Box Frame** | Multi-Line | **Single-Line** | High-density nested borders clip on CGA. Enforce 1-pixel solid lines. |
| **Corner Corner Radius** | 4 (Radius) | **2 (Radius)** | Represented as a single 45-degree clipped pixel at vertices. |

---

## 5. Visual Palette Mapping (CGA Mode 4)

To accurately emulate the high-contrast, professional monochrome finish of the Macintosh's built-in 9-inch CRT on a 4-color CGA setup, software engineers must configure the system to run **CGA Mode 4, Palette 1 (High Intensity)**.

### Color Register Assignments

1. **Color 0 (Background):** `0x00` - Solid Black
2. **Color 1 (Accents):** `0x0B` - Light Cyan (Used for desktop background fill patterns, window inactive states, or checkerboard dithering).
3. **Color 2 (Alerts):** `0x0D` - Light Magenta (Reserved strictly for active contextual selections, highlighting, modal warning frames, or critical alerts).
4. **Color 3 (Foreground):** `0x0F` - Solid White (Used for main window canvas regions, text string typography, and internal control structures).

### Emulated UI Palette Application Matrix
* **Desktop Background (The Workspace):** Fill with a 50% dither pattern alternating between **Color 0 (Black)** and **Color 1 (Light Cyan)** to mimic the classic Mac desktop pattern without causing chromatic fatigue.
* **Active Window Window Body:** Solid **Color 3 (White)** frame fill.
* **Window Title / Borders / Text:** Rendered using crisp, high-contrast **Color 0 (Black)** lines.
* **Selected Text / Highlighted Icons:** Perform an immediate bitwise XOR operation or swap **Color 3 (White)** canvas space with **Color 2 (Light Magenta)** to cleanly isolate user focus.
