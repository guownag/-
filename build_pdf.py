# -*- coding: utf-8 -*-
"""Generate a polished PDF for the embedded-systems learning path."""

import os
import re

from reportlab.lib.pagesizes import A4
from reportlab.lib.colors import HexColor, white
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfgen import canvas as rl_canvas


# ---------------------------------------------------------------- fonts
FONT_DIR = "C:/Windows/Fonts"
pdfmetrics.registerFont(TTFont("YH", os.path.join(FONT_DIR, "msyh.ttc"), subfontIndex=0))
pdfmetrics.registerFont(TTFont("YHB", os.path.join(FONT_DIR, "msyhbd.ttc"), subfontIndex=0))
pdfmetrics.registerFont(TTFont("YHL", os.path.join(FONT_DIR, "msyhl.ttc"), subfontIndex=0))
REG, BOLD, LIGHT = "YH", "YHB", "YHL"

# ---------------------------------------------------------------- palette
INK = HexColor("#23282E")
MUTED = HexColor("#5C6672")
FAINT = HexColor("#8B95A1")
PRIMARY = HexColor("#0E7C7B")
PRIMARY_DARK = HexColor("#0A5B5A")
PRIMARY_LIGHT = HexColor("#E2F1F0")
PRIMARY_MID = HexColor("#4FB0AE")
ACCENT = HexColor("#E08A32")
ACCENT_LIGHT = HexColor("#FBEEDA")
CARD_BORDER = HexColor("#E3E9E9")
RULE = HexColor("#EDF1F1")
SOFT = HexColor("#F6F8F8")

PAGE_W, PAGE_H = A4
LEFT = 56.0
RIGHT = 56.0
CONTENT_W = PAGE_W - LEFT - RIGHT
TOP_MARGIN = 64.0
BOTTOM_MARGIN = 62.0


# ---------------------------------------------------------------- text utils
TOKEN_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9_\-./+#%]*|\s+|.", re.S)


def tokenize(text):
    return TOKEN_RE.findall(text)


def wrap_text(text, font, size, max_w):
    """Wrap text that mixes CJK characters and latin words."""
    lines = []
    for para in text.split("\n"):
        cur = ""
        for tok in tokenize(para):
            if not tok.strip():
                if cur and pdfmetrics.stringWidth(cur + " ", font, size) <= max_w:
                    cur += " "
                continue
            trial = cur + tok
            if cur == "" or pdfmetrics.stringWidth(trial, font, size) <= max_w:
                cur = trial
            else:
                lines.append(cur.rstrip())
                cur = tok
        lines.append(cur.rstrip())
    return lines or [""]


def sw(text, font, size):
    return pdfmetrics.stringWidth(text, font, size)


# ---------------------------------------------------------------- primitives
def text(c, x, y, s, font=REG, size=10, color=INK):
    c.setFont(font, size)
    c.setFillColor(color)
    c.drawString(x, y, s)


def text_right(c, x, y, s, font=REG, size=10, color=INK):
    c.setFont(font, size)
    c.setFillColor(color)
    c.drawRightString(x, y, s)


def text_center(c, cx, y, s, font=REG, size=10, color=INK):
    c.setFont(font, size)
    c.setFillColor(color)
    c.drawCentredString(cx, y, s)


def para(c, x, y, lines, font=REG, size=10, color=INK, leading=15):
    c.setFont(font, size)
    c.setFillColor(color)
    for ln in lines:
        c.drawString(x, y, ln)
        y -= leading
    return y


def block(c, x, top, lines, font=REG, size=10, color=INK, leading=15):
    """Draw a wrapped text block anchored at its top edge; return its bottom."""
    y = top - size * 0.88
    c.setFont(font, size)
    c.setFillColor(color)
    for ln in lines:
        c.drawString(x, y, ln)
        y -= leading
    return top - leading * len(lines)


def pill(c, x, y_bottom, label, font=REG, size=8.5,
         bg=PRIMARY_LIGHT, fg=PRIMARY_DARK, pad_x=7.5, height=19.0):
    w = sw(label, font, size) + pad_x * 2
    c.setFillColor(bg)
    c.roundRect(x, y_bottom, w, height, height / 2.0, stroke=0, fill=1)
    text(c, x + pad_x, y_bottom + height / 2.0 - size * 0.35, label, font, size, fg)
    return w


def hairline(c, x1, x2, y, color=RULE, width=0.7):
    c.setStrokeColor(color)
    c.setLineWidth(width)
    c.line(x1, y, x2, y)


def circle_badge(c, x, y_top, d, label, fill, fg=white, font=BOLD, size=13):
    c.setFillColor(fill)
    c.circle(x + d / 2.0, y_top - d / 2.0, d / 2.0, stroke=0, fill=1)
    text_center(c, x + d / 2.0, y_top - d / 2.0 - size * 0.36, label, font, size, fg)


# ---------------------------------------------------------------- content
STAGES = [
    dict(
        n=1,
        title="编程与硬件基础",
        caption="C 语言、数字 / 模拟电路、调试工具",
        goal="能读懂电路，写得出规范的 C 代码。",
        points=[
            ("C 语言", "指针、结构体、位运算、内存布局、volatile / const"),
            ("数字电路", "进制与逻辑门、组合与时序逻辑、GPIO 电平概念"),
            ("模拟基础", "上下拉电阻、分压、三极管开关、简单 RC 滤波"),
            ("工具", "万用表、示波器看波形，会读原理图引脚定义"),
        ],
        tags=["C 语言", "数字电路", "模拟基础"],
    ),
    dict(
        n=2,
        title="单片机入门（STM32）",
        caption="STM32 外设、HAL / CubeMX、在线调试",
        goal="打通「代码 -> 控制硬件」的完整链路。",
        points=[
            ("选型", "STM32F103 / F4 系列，资料最多、生态最全"),
            ("外设", "GPIO、外部中断、定时器 PWM / 输入捕获、USART、ADC / DAC、I2C、SPI"),
            ("开发", "先寄存器 / 标准库理解原理，再上 HAL + CubeMX 提效"),
            ("调试", "CubeIDE + ST-Link 单步调试，比串口打印快得多"),
        ],
        tags=["STM32", "HAL", "CubeMX", "ST-Link"],
    ),
    dict(
        n=3,
        title="RTOS 与工程化",
        caption="FreeRTOS / RT-Thread、模块分层、Git",
        goal="从「裸机 while(1)」升级到能管理多任务。",
        points=[
            ("RTOS", "FreeRTOS 或 RT-Thread：任务调度、队列、信号量、互斥锁、事件组、软定时器"),
            ("中断与 RTOS", "分清哪些能在中断里做，哪些必须丢给任务处理"),
            ("工程习惯", "模块分层、日志系统、Git 版本控制、Makefile / CMake"),
            ("排障能力", "会用栈水位、CPU 占用与死锁检测定位问题"),
        ],
        tags=["FreeRTOS", "RT-Thread", "Git", "CMake"],
    ),
    dict(
        n=4,
        title="通信与系统能力",
        caption="I2C / SPI / UART、CAN、Modbus、MQTT",
        goal="把多个设备连起来，具备系统级视角。",
        points=[
            ("板级总线", "I2C、SPI、UART 深入，解决时序问题与现场排障"),
            ("网络协议", "CAN（车载 / 工业）、Modbus（工控）、MQTT + WiFi / 4G（物联网）"),
            ("存储与文件", "Flash / EEPROM、外部 SD 卡、FatFs 文件系统"),
            ("低功耗设计", "睡眠模式、时钟树配置、功耗测量与优化"),
        ],
        tags=["CAN", "Modbus", "MQTT", "FatFs"],
    ),
    dict(
        n=5,
        title="嵌入式 Linux（分水岭）",
        caption="系统编程、驱动开发、Buildroot / Yocto",
        goal="面向系统与底层方向，掌握整机软件栈。",
        points=[
            ("Linux 使用", "命令行、Shell、Git、交叉编译工具链"),
            ("系统编程", "文件 IO、进程与线程、进程间通信、socket 网络编程"),
            ("驱动开发", "字符设备、设备树、GPIO / I2C / SPI 子系统、中断处理"),
            ("系统构建", "Buildroot / Yocto，uboot + kernel + rootfs 完整流程"),
        ],
        tags=["Linux", "驱动开发", "Buildroot", "Yocto"],
    ),
    dict(
        n=6,
        title="项目实战（最关键）",
        caption="从智能小车到物联网网关与板卡项目",
        goal="用项目把前面所有知识真正串起来。",
        points=[
            ("智能小车 / 平衡车", "电机 PWM、编码器测速、PID 控制、传感器融合"),
            ("环境监测站", "多传感器采集 + 显示屏 + 数据上传云端"),
            ("物联网网关", "RTOS + 协议栈 + 云端对接与远程升级"),
            ("Linux 板卡项目", "摄像头采集 + 驱动开发 + 网络传输"),
        ],
        tags=["PID", "传感器融合", "物联网", "驱动开发"],
    ),
]

DIRECTIONS = [
    ("偏控制与实时", "单片机 + RTOS 深入，做电机、电源、工业控制类岗位"),
    ("偏产品与联网", "物联网 + 通信协议 + 云端，做智能硬件与 IoT 产品"),
    ("偏底层与系统", "嵌入式 Linux + 驱动，做系统、BSP、底层平台方向"),
    ("偏算法与信号", "传感器融合、控制算法、数字信号处理方向"),
]

RESOURCES = [
    "芯片数据手册与参考手册当作字典常用，比任何教程都权威。",
    "正点原子 / 野火的 STM32 教程适合打基础，配套开发板边学边练。",
    "《C 和指针》《深入理解计算机系统》补齐语言与系统底层能力。",
    "Linux 方向看《Linux 设备驱动开发》与内核官方文档。",
]

ADVICE = [
    "每个阶段都要有一个可交付的小项目，避免只看不练。",
    "遇到问题先查数据手册与官方文档，再检索社区与开源代码。",
    "持续沉淀自己的代码库与学习笔记，长期会产生复利。",
]

PITFALLS = [
    "只看视频不动手：一定要配合开发板写代码、调硬件。",
    "贪多求全：先在一个方向上挖深，再横向扩展知识面。",
    "忽视手册：数据手册与参考手册永远是最权威的第一手资料。",
    "跳过 C 语言基础：指针与内存是后面一切内容的地基。",
]


# ---------------------------------------------------------------- document
class Doc:
    def __init__(self, path, total_pages=None):
        self.c = rl_canvas.Canvas(path, pagesize=A4)
        self.c.setTitle("嵌入式学习路径")
        self.c.setAuthor("Codex")
        self.c.setSubject("从零到独立项目的六阶段进阶指南")
        self.page = 0
        self.total = total_pages
        self.y = 0.0

    # -- page helpers
    def new_page(self, header=True):
        if self.page:
            self.c.showPage()
        self.page += 1
        if header:
            self._running_header()
            self.y = PAGE_H - 80.0
        else:
            self.y = PAGE_H

    def _running_header(self):
        c = self.c
        text(c, LEFT, PAGE_H - 42, "嵌入式学习路径", REG, 8.5, FAINT)
        label = "%d" % self.page if self.total is None else "%d / %d" % (self.page, self.total)
        text_right(c, PAGE_W - RIGHT, PAGE_H - 42, label, REG, 8.5, FAINT)
        hairline(c, LEFT, PAGE_W - RIGHT, PAGE_H - 52, RULE, 0.7)

    def ensure(self, space):
        if self.y - space < BOTTOM_MARGIN:
            self.new_page()


# ---------------------------------------------------------------- cover
def draw_cover(doc):
    c = doc.c
    panel_h = 336.0
    panel_top = PAGE_H

    c.setFillColor(SOFT)
    c.rect(0, 0, PAGE_W, PAGE_H, stroke=0, fill=1)

    c.setFillColor(PRIMARY)
    c.rect(0, panel_top - panel_h, PAGE_W, panel_h, stroke=0, fill=1)

    # faint pcb-style traces
    c.saveState()
    c.setStrokeAlpha(0.28)
    c.setFillAlpha(0.28)
    c.setStrokeColor(PRIMARY_MID)
    c.setFillColor(PRIMARY_MID)
    c.setLineWidth(1.1)
    traces = [
        [(404, panel_top - 258), (462, panel_top - 258), (462, panel_top - 294), (540, panel_top - 294)],
        [(420, panel_top - 322), (486, panel_top - 322), (486, panel_top - 272), (546, panel_top - 272)],
        [(388, panel_top - 300), (436, panel_top - 300), (436, panel_top - 330), (516, panel_top - 330)],
    ]
    for tr in traces:
        p = c.beginPath()
        p.moveTo(*tr[0])
        for pt in tr[1:]:
            p.lineTo(*pt)
        c.drawPath(p, stroke=1, fill=0)
        for pt in (tr[0], tr[-1]):
            c.circle(pt[0], pt[1], 2.6, stroke=0, fill=1)
            c.circle(pt[0], pt[1], 6.0, stroke=1, fill=0)
    c.restoreState()

    text(c, LEFT, panel_top - 78, "L E A R N I N G   R O A D M A P", BOLD, 9, HexColor("#8FD2D0"))
    text(c, LEFT, panel_top - 146, "嵌入式学习路径", BOLD, 42, white)

    c.setFillColor(ACCENT)
    c.rect(LEFT, panel_top - 168, 72, 5, stroke=0, fill=1)

    text(c, LEFT, panel_top - 206, "从零到独立项目的六阶段进阶指南", REG, 16, HexColor("#D8EEED"))

    desc = "覆盖 C 语言与硬件基础、STM32 单片机、RTOS、通信协议、嵌入式 Linux 与项目实战，"
    desc += "每一阶段都给出清晰目标与关键知识点。"
    lines = wrap_text(desc, REG, 10.5, 400)
    para(c, LEFT, panel_top - 236, lines, REG, 10.5, HexColor("#A3D5D4"), 16)

    # stage overview
    y = panel_top - panel_h - 44
    text(c, LEFT, y, "六 个 阶 段", BOLD, 9.5, PRIMARY)
    y -= 12
    hairline(c, LEFT, PAGE_W - RIGHT, y, RULE, 0.8)
    y -= 24

    for i, st in enumerate(STAGES):
        circle_badge(c, LEFT, y, 26, str(st["n"]), PRIMARY, size=11.5)
        text(c, LEFT + 40, y - 10, st["title"], BOLD, 12.5, INK)
        text(c, LEFT + 40, y - 26, st["caption"], REG, 9.5, MUTED)
        if i < len(STAGES) - 1:
            hairline(c, LEFT, PAGE_W - RIGHT, y - 40, RULE, 0.7)
        y -= 44

    hairline(c, LEFT, PAGE_W - RIGHT, 74, RULE, 0.7)
    text(c, LEFT, 58, "嵌入式学习路径 · 学习指南", REG, 8.5, FAINT)
    text_right(c, PAGE_W - RIGHT, 58, "2026-09-11", REG, 8.5, FAINT)


# ---------------------------------------------------------------- cards
def draw_card(doc, stage):
    c = doc.c
    pad = 16.5
    x = LEFT
    w = CONTENT_W
    badge_d = 30.0
    g1, g2, g3 = 10.0, 14.0, 9.0
    lead, goal_lead, bullet_gap = 15.3, 14.5, 8.5

    goal_lines = wrap_text(stage["goal"], REG, 10.5, w - pad * 2)
    bullet_blocks = []
    for label, txt in stage["points"]:
        label_w = sw(label, BOLD, 10.5)
        avail = w - pad * 2 - 13 - label_w - 8
        blines = wrap_text(txt, REG, 10.5, avail)
        bullet_blocks.append((label, label_w, blines))

    # tag rows
    chip_gap, chip_h = 7.0, 19.0
    tag_rows, cur = [], []
    cur_w = 0.0
    for t in stage["tags"]:
        cw = sw(t, REG, 8.5) + 15
        if cur and cur_w + chip_gap + cw > w - pad * 2:
            tag_rows.append(cur)
            cur, cur_w = [], 0.0
        cur.append((t, cw))
        cur_w += cw + (chip_gap if len(cur) > 1 else 0)
    if cur:
        tag_rows.append(cur)

    goal_h = len(goal_lines) * goal_lead
    bullets_h = sum(len(b[2]) * lead for b in bullet_blocks) + bullet_gap * (len(bullet_blocks) - 1)
    tags_h = len(tag_rows) * chip_h + (len(tag_rows) - 1) * 6.0
    height = (pad + badge_d + g1 + goal_h + g2 + bullets_h + g3 + tags_h + pad)

    doc.ensure(height + 6)
    y_top = doc.y

    # card surface
    c.setFillColor(white)
    c.setStrokeColor(CARD_BORDER)
    c.setLineWidth(0.8)
    c.roundRect(x, y_top - height, w, height, 8, stroke=1, fill=1)

    # clipped left accent band
    c.saveState()
    p = c.beginPath()
    p.roundRect(x, y_top - height, w, height, 8)
    c.clipPath(p, stroke=0, fill=0)
    c.setFillColor(PRIMARY)
    c.rect(x, y_top - height, 5, height, stroke=0, fill=1)
    c.restoreState()

    top = y_top - pad
    circle_badge(c, x + pad + 4, top, badge_d, str(stage["n"]), ACCENT, size=14)
    text(c, x + pad + 4 + badge_d + 12, top - badge_d / 2.0 - 5.5, stage["title"], BOLD, 15, INK)
    text_right(c, x + w - pad, top - badge_d / 2.0 - 4.5, stage["caption"], REG, 8.5, FAINT)

    top = top - badge_d - g1
    goal_bottom = block(c, x + pad, top, goal_lines, REG, 10.5, PRIMARY_DARK, goal_lead)

    hairline(c, x + pad, x + w - pad, goal_bottom - g2 / 2.0, RULE, 0.7)
    top = goal_bottom - g2

    for i, (label, label_w, blines) in enumerate(bullet_blocks):
        c.setFillColor(PRIMARY_MID)
        c.circle(x + pad + 5, top - 5.5, 2.1, stroke=0, fill=1)
        block(c, x + pad + 16, top, [label], BOLD, 10.5, INK, lead)
        block(c, x + pad + 16 + label_w + 8, top, blines, REG, 10.5, HexColor("#414A54"), lead)
        top -= len(blines) * lead
        if i < len(bullet_blocks) - 1:
            top -= bullet_gap

    top -= g3
    chip_y = top - chip_h
    for row in tag_rows:
        cx = x + pad
        for t, cw in row:
            pill(c, cx, chip_y, t, REG, 8.5, ACCENT_LIGHT, HexColor("#9A5A18"))
            cx += cw + chip_gap
        chip_y -= chip_h + 6

    doc.y = y_top - height - 16


# ---------------------------------------------------------------- closing page
def draw_section_head(doc, title, gap_after=22):
    c = doc.c
    doc.ensure(60)
    y = doc.y
    c.setFillColor(ACCENT)
    c.roundRect(LEFT, y - 15, 4, 16, 2, stroke=0, fill=1)
    text(c, LEFT + 14, y - 13.5, title, BOLD, 15, INK)
    doc.y = y - gap_after


def draw_directions(doc):
    c = doc.c
    gap = 16.0
    cw = (CONTENT_W - gap) / 2.0
    ch = 78.0
    y = doc.y
    for i, (title, desc) in enumerate(DIRECTIONS):
        col = i % 2
        row = i // 2
        x = LEFT + col * (cw + gap)
        top = y - row * (ch + 14)
        c.setFillColor(white)
        c.setStrokeColor(CARD_BORDER)
        c.setLineWidth(0.8)
        c.roundRect(x, top - ch, cw, ch, 8, stroke=1, fill=1)
        c.setFillColor(PRIMARY_LIGHT)
        c.roundRect(x, top - ch, cw, 5, 2, stroke=0, fill=1)
        text(c, x + 16, top - 30, title, BOLD, 11.5, PRIMARY_DARK)
        lines = wrap_text(desc, REG, 9.5, cw - 32)
        para(c, x + 16, top - 50, lines[:3], REG, 9.5, MUTED, 13.5)
    doc.y = y - 2 * ch - 14 - 6


def draw_list(doc, items, leading=14.5, gap=12.0):
    c = doc.c
    for item in items:
        doc.ensure(48)
        y = doc.y
        lines = wrap_text(item, REG, 10.5, CONTENT_W - 18)
        c.setFillColor(PRIMARY_MID)
        c.circle(LEFT + 3, y + 3.7, 2.1, stroke=0, fill=1)
        end = para(c, LEFT + 18, y, lines, REG, 10.5, HexColor("#414A54"), leading)
        doc.y = end - gap
    doc.y += gap - 6


def draw_advice(doc):
    c = doc.c
    doc.ensure(120)
    y = doc.y
    lines_all = []
    for item in ADVICE:
        lines_all.append(wrap_text(item, REG, 10.5, CONTENT_W - 18))
    height = 22 + sum(len(l) * 15.0 + 9 for l in lines_all) + 6
    c.setFillColor(SOFT)
    c.setStrokeColor(CARD_BORDER)
    c.setLineWidth(0.8)
    c.roundRect(LEFT, y - height, CONTENT_W, height, 8, stroke=1, fill=1)
    text(c, LEFT + 20, y - 26, "学习建议", BOLD, 12, PRIMARY_DARK)
    cy = y - 48
    for lines in lines_all:
        c.setFillColor(ACCENT)
        c.circle(LEFT + 22, cy + 3.2, 2.1, stroke=0, fill=1)
        cy = para(c, LEFT + 36, cy, lines, REG, 10.5, HexColor("#414A54"), 15.0)
        cy -= 9
    doc.y = y - height - 10


def draw_closing(doc):
    draw_section_head(doc, "方向选择", 28)
    draw_directions(doc)
    doc.y -= 20
    draw_section_head(doc, "推荐资源", 30)
    draw_list(doc, RESOURCES)
    doc.y -= 8
    draw_section_head(doc, "常见误区", 30)
    draw_list(doc, PITFALLS)
    doc.y -= 8
    draw_advice(doc)


# ---------------------------------------------------------------- build
def build(path, total_pages=None):
    doc = Doc(path, total_pages)

    doc.new_page(header=False)
    draw_cover(doc)

    doc.new_page()
    for st in STAGES:
        doc.ensure(200)
        draw_card(doc, st)

    doc.new_page()
    draw_closing(doc)

    doc.c.showPage()
    doc.c.save()
    return doc.page


def main():
    out_dir = os.path.join("output", "pdf")
    os.makedirs(out_dir, exist_ok=True)
    tmp_dir = os.path.join("tmp", "pdfs")
    os.makedirs(tmp_dir, exist_ok=True)

    tmp = os.path.join(tmp_dir, "_pass1.pdf")
    n = build(tmp, None)
    final = os.path.join(out_dir, "嵌入式学习路径.pdf")
    build(final, n)
    print("pages:", n)
    print("output:", os.path.abspath(final))


if __name__ == "__main__":
    main()
