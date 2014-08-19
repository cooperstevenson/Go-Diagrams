#!/usr/bin/python -tt

import sgflib, sgfboard, optparse, sys, math

# SVG Templates {{{
def triangle_points(r=3.5, yoff=0):
    rp = r/math.cos(math.radians(60))
    x = r*math.tan(math.radians(60))
    points = "%s,%s %s,%s %s,%s" % (0,-rp+yoff, x,r+yoff, -x,r+yoff)
    return points

SVG_HEAD = '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:xlink="http://www.w3.org/1999/xlink"
   version="1.1"
   width="%(width)d" height="%(height)d">
'''

SVG_DEFS = '''
<defs>
    <style type="text/css"><![CDATA[
        %(sgf_styles)s
    ]]></style>
    %(sgf_defs)s
</defs>
'''

SVG_BODY = '''
<g id="container" transform="translate(-%(crop_left)s, -%(crop_top)s)">
    <g id="board">
    %(board)s
    </g>
    <g id="stones">
    %(stones)s
    </g>
    <g id="markers">
    %(markers)s
    </g>
</g>
'''

SVG_FOOT = '</svg>\n'
# }}}

THEMES = {

    'bare': { # {{{
        'stone_size': 31,
        'arrowhead_fix': 3,
        'board_padding': 10.0,
        'hoshi_size': 5.0,
        'font_size': 16.0,
        'line_width': 5.0,
        'font_family': '"Droid Sans","Trebuchet MS",Arial,Helvetica,sans-serif',
        'sgf_styles': '''
            circle.stoneB {
                fill: black;
            }

            circle.stoneW {
                fill: white;
                stroke: black;
                stroke-width: 2;
            }

            circle.hoshi {
                fill: black;
            }

            #boardBackground {
                fill: white;
            }

            .grid {
                stroke: black;
                stroke-width: 1.0;
                stroke-linecap: square;
            }

            .markerCR, .markerMA, .markerSQ, .markerTR {
                stroke-width: 2.0;
                fill-opacity: 0.0;
            }

            .markerCR_W, .markerMA_W, .markerSQ_W, .markerTR_W,
            .markerCR_None, .markerMA_None, .markerSQ_None, .markerTR_None {
                stroke: black;
            }

            .markerCR_B, .markerMA_B, .markerSQ_B, .markerTR_B {
                stroke: white;
            }

            .markerMA {
                stroke-linecap: square;
            }

            .markerTR {
                stroke-linejoin: bevel;
            }

            .None .markerCurrent, .W .markerCurrent {
                fill: black;
            }

            .B .markerCurrent {
                fill: white;
            }

            .markerTB {
                fill: black;
            }

            .markerTW {
                fill: white;
                stroke-width: 1px;
                stroke: #7f7f7f;
            }

            .markerSL {
                fill: #ffffff;
                fill-opacity: 0.5;
            }

            .markerDD {
                fill: #000000;
                fill-opacity: 0.5;
            }

            .markerLB {
                font-family: %(font_family)s;
                font-size: %(font_size)spx;
                font-weight: bold;
                font-stretch: condensed;
                text-anchor: middle;
            }

            .markerLB_W_fg, .markerLB.None_fg {
                fill: black;
            }

            .markerLB_B_fg {
                fill: white;
            }

            .markerLB_W_bg, .markerLB_None_bg {
                fill: white;
                stroke: white;
                stroke-width: 2;
            }

            .markerLB_B_bg {
                fill: black;
                stroke: black;
                stroke-width: 2;
            }

            .markerLN, .markerAR {
                stroke: #7f7f7f;
                stroke-width: %(line_width)s;
                stroke-linecap: round;
            }

            #arrHead {
                fill: #7f7f7f;
            }

            .markerAR {
                marker-end: url(#arrHead);
            }
        ''',
        'sgf_defs': '''
            <symbol id="hoshi" viewBox="-2 -2 4 4">
                <circle class="hoshi" r="2" />
            </symbol>

            <symbol id="stoneB" viewBox="-15.5 -15.5 31 31">
                <circle class="stoneB" r="15.5" />
            </symbol>
            <symbol id="stoneW" viewBox="-15.5 -15.5 31 31">
                <circle class="stoneW" r="14.5" />
            </symbol>

            <symbol id="markerCR" viewBox="-15.5 -15.5 31 31">
                <circle class="markerCR" r="7.5" />
            </symbol>
            <symbol id="markerMA" viewBox="-15.5 -15.5 31 31">
                <line class="markerMA" x1="-6.5" y1="-6.5" x2="6.5" y2="6.5" />
                <line class="markerMA" x1="-6.5" y1="6.5" x2="6.5" y2="-6.5" />
            </symbol>
            <symbol id="markerSQ" viewBox="-15.5 -15.5 31 31">
                <rect class="markerSQ" x="-7.5" y="-7.5" width="15" height="15" />
            </symbol>
            <symbol id="markerTR" viewBox="-15.5 -15.5 31 31">
                <polygon class="markerTR" points="''' + triangle_points(5.5) + '''" />
            </symbol>
            <symbol id="markerCurrent" viewBox="-15.5 -15.5 31 31">
                <rect class="markerCurrent" x="-5.5" y="-5.5" width="11" height="11" />
            </symbol>

            <symbol id="markerTB" viewBox="-15.5 -15.5 31 31">
                <circle class="markerTB" r="5.5" />
            </symbol>
            <symbol id="markerTW" viewBox="-9.5 -9.5 19 19">
                <circle class="markerTW" r="5" />
            </symbol>

            <symbol id="markerSL" viewBox="0 0 1 1">
                <rect class="markerSL" x="0" y="0" width="1" height="1" />
            </symbol>
            <symbol id="markerDD" viewBox="0 0 1 1">
                <rect class="markerDD" x="0" y="0" width="1" height="1" />
            </symbol>

            <marker id="arrHead" orient="auto" refX="1" refY="2"
                    markerWidth="5" markerHeight="5">
                <polygon points="4,2 0,4 1,2 0,0" />
            </marker>
        '''
    }, # }}}

    'default': { # {{{
        'stone_size': 19,
        'arrowhead_fix': 3,
        'board_padding': 5.0,
        'hoshi_size': 4.0,
        'font_size': 12.0,
        'line_width': 3.0,
        'font_family': '"Droid Sans","Trebuchet MS",Arial,Helvetica,sans-serif',
        'sgf_styles': '''
            circle.stoneB {
                fill: url(#blackStoneGradient);
                stroke: #000000;
                stroke-width: 1;
            }

            circle.stoneW {
                fill: url(#whiteStoneGradient);
                stroke: #808080;
                stroke-width: 1;
            }

            circle.hoshi {
                fill: #705f36;
            }

            #boardBackground {
                fill: #ddbc6b;
            }

            .grid {
                stroke: #ae9454;
                stroke-width: 1.0;
                stroke-linecap: square;
            }

            .markerCR, .markerMA, .markerSQ, .markerTR {
                stroke-width: 1.0;
                fill-opacity: 0.0;
            }

            .markerMA {
                stroke-linecap: square;
            }

            .markerTR {
                stroke-linejoin: bevel;
            }

            .markerCR_W, .markerMA_W, .markerSQ_W, .markerTR_W,
            .markerCR_None, .markerMA_None, .markerSQ_None, .markerTR_None {
                stroke: black;
            }

            .markerCR_B, .markerMA_B, .markerSQ_B, .markerTR_B {
                stroke: white;
            }

            .markerCurrent {
                fill: #ff0000;
            }

            .markerTB {
                fill: black;
            }

            .markerTW {
                fill: white;
            }

            .markerSL {
                fill: #ffffff;
                fill-opacity: 0.4;
            }

            .markerDD {
                fill: #000000;
                fill-opacity: 0.4;
            }

            .markerLB {
                font-family: %(font_family)s;
                font-size: %(font_size)spx;
                font-stretch: condensed;
                text-anchor: middle;
            }

            .markerLB_W_fg, .markerLB.None_fg {
                fill: black;
            }

            .markerLB_B_fg {
                fill: white;
            }

            .markerLB_None_bg {
                fill: #ddbc6b;
                stroke: #ddbc6b;
                stroke-width: 3;
            }

            .markerLB_B_bg {
                fill: black;
                stroke: black;
                stroke-width: 1;
            }

            .markerLB_W_bg {
                fill: white;
                stroke: white;
                stroke-width: 1;
            }

            .markerLN, .markerAR {
                stroke: #0066bb;
                stroke-width: %(line_width)s;
                stroke-linecap: round;
            }

            #arrHead {
                fill: #0066bb;
            }

            .markerAR {
                marker-end: url(#arrHead);
            }
        ''',
        'sgf_defs': '''
            <symbol id="hoshi" viewBox="-2 -2 4 4">
                <circle class="hoshi" r="2" />
            </symbol>

            <radialGradient id="blackStoneGradient"
                cx="-3" cy="-3" fx="-3" fy="-3" r="11" gradientUnits="userSpaceOnUse">
                <stop stop-color="#505050" stop-opacity="1.0" offset="0.0" />
                <stop stop-color="#282828" stop-opacity="1.0" offset="0.6" />
                <stop stop-color="#000000" stop-opacity="1.0" offset="1.0" />
            </radialGradient>
            <radialGradient id="whiteStoneGradient"
                cx="-3" cy="-3" fx="-3" fy="-3" r="11" gradientUnits="userSpaceOnUse">
                <stop stop-color="#ffffff" stop-opacity="1.0" offset="0.0" />
                <stop stop-color="#ffffff" stop-opacity="1.0" offset="0.3" />
                <stop stop-color="#d0d0d0" stop-opacity="1.0" offset="1.0" />
            </radialGradient>

            <symbol id="stoneB" viewBox="-9.5 -9.5 19 19">
                <circle class="stoneB" r="9" />
            </symbol>
            <symbol id="stoneW" viewBox="-9.5 -9.5 19 19">
                <circle class="stoneW" r="9" />
            </symbol>

            <symbol id="markerCR" viewBox="-9.5 -9.5 19 19">
                <circle class="markerCR" r="4" />
            </symbol>
            <symbol id="markerMA" viewBox="-9.5 -9.5 19 19">
                <line class="markerMA" x1="-4" y1="-4" x2="4" y2="4" />
                <line class="markerMA" x1="-4" y1="4" x2="4" y2="-4" />
            </symbol>
            <symbol id="markerSQ" viewBox="-9.5 -9.5 19 19">
                <rect class="markerSQ" x="-4" y="-4" width="8" height="8" />
            </symbol>
            <symbol id="markerTR" viewBox="-9.5 -9.5 19 19">
                <polygon class="markerTR" points="''' + triangle_points(3) + '''" />
            </symbol>
            <symbol id="markerCurrent" viewBox="-9.5 -9.5 19 19">
                <rect class="markerCurrent" x="-3.5" y="-3.5" width="7" height="7" />
            </symbol>

            <symbol id="markerTB" viewBox="-9.5 -9.5 19 19">
                <circle class="markerTB" r="3.5" />
            </symbol>
            <symbol id="markerTW" viewBox="-9.5 -9.5 19 19">
                <circle class="markerTW" r="3.5" />
            </symbol>

            <symbol id="markerSL" viewBox="0 0 1 1">
                <rect class="markerSL" x="0" y="0" width="1" height="1" />
            </symbol>
            <symbol id="markerDD" viewBox="0 0 1 1">
                <rect class="markerDD" x="0" y="0" width="1" height="1" />
            </symbol>

            <marker id="arrHead" orient="auto" refX="1" refY="2"
                    markerWidth="5" markerHeight="5">
                <polygon points="4,2 0,4 1,2 0,0" />
            </marker>
        '''
    }, # }}}

    'tiny': { # {{{
        'stone_size': 13,
        'arrowhead_fix': 3,
        'board_padding': 3.0,
        'hoshi_size': 3.0,
        'font_size': 9.0,
        'line_width': 2.0,
        'font_family': '"Droid Sans","Trebuchet MS",Arial,Helvetica,sans-serif',
        'sgf_styles': '''
            circle.stoneB {
                fill: url(#blackStoneGradient);
                stroke: #000000;
                stroke-width: 1;
            }

            circle.stoneW {
                fill: url(#whiteStoneGradient);
                stroke: #808080;
                stroke-width: 1;
            }

            circle.hoshi {
                fill: #705f36;
            }

            #boardBackground {
                fill: #ddbc6b;
            }

            .grid {
                stroke: #ae9454;
                stroke-width: 1.0;
                stroke-linecap: square;
            }

            .markerCR, .markerMA, .markerSQ, .markerTR {
                stroke-width: 1.0;
                fill-opacity: 0.0;
            }

            .markerMA {
                stroke-linecap: square;
            }

            .markerTR {
                stroke-linejoin: bevel;
            }

            .markerCR_W, .markerMA_W, .markerSQ_W, .markerTR_W,
            .markerCR_None, .markerMA_None, .markerSQ_None, .markerTR_None {
                stroke: black;
            }

            .markerCR_B, .markerMA_B, .markerSQ_B, .markerTR_B {
                stroke: white;
            }

            .markerCurrent {
                fill: #ff0000;
            }

            .markerTB {
                fill: black;
            }

            .markerTW {
                fill: white;
            }

            .markerSL {
                fill: #ffffff;
                fill-opacity: 0.4;
            }

            .markerDD {
                fill: #000000;
                fill-opacity: 0.4;
            }

            .markerLB {
                font-family: %(font_family)s;
                font-size: %(font_size)spx;
                font-stretch: condensed;
                text-anchor: middle;
            }

            .markerLB_W_fg, .markerLB.None_fg {
                fill: black;
            }

            .markerLB_B_fg {
                fill: white;
            }

            .markerLB_None_bg {
                fill: #ddbc6b;
                stroke: #ddbc6b;
                stroke-width: 3;
            }

            .markerLB_B_bg {
                fill: black;
                stroke: black;
                stroke-width: 1;
            }

            .markerLB_W_bg {
                fill: white;
                stroke: white;
                stroke-width: 1;
            }

            .markerLN, .markerAR {
                stroke: #0066bb;
                stroke-width: %(line_width)s;
                stroke-linecap: round;
            }

            #arrHead {
                fill: #0066bb;
            }

            .markerAR {
                marker-end: url(#arrHead);
            }
        ''',
        'sgf_defs': '''
            <symbol id="hoshi" viewBox="-2 -2 4 4">
                <circle class="hoshi" r="2" />
            </symbol>

            <radialGradient id="blackStoneGradient"
                cx="-2" cy="-2" fx="-2" fy="-2" r="7.5" gradientUnits="userSpaceOnUse">
                <stop stop-color="#505050" stop-opacity="1.0" offset="0.0" />
                <stop stop-color="#282828" stop-opacity="1.0" offset="0.6" />
                <stop stop-color="#000000" stop-opacity="1.0" offset="1.0" />
            </radialGradient>
            <radialGradient id="whiteStoneGradient"
                cx="-2" cy="-2" fx="-2" fy="-2" r="7.5" gradientUnits="userSpaceOnUse">
                <stop stop-color="#ffffff" stop-opacity="1.0" offset="0.0" />
                <stop stop-color="#ffffff" stop-opacity="1.0" offset="0.3" />
                <stop stop-color="#d0d0d0" stop-opacity="1.0" offset="1.0" />
            </radialGradient>

            <symbol id="stoneB" viewBox="-6.5 -6.5 13 13">
                <circle class="stoneB" r="6" />
            </symbol>
            <symbol id="stoneW" viewBox="-6.5 -6.5 13 13">
                <circle class="stoneW" r="6" />
            </symbol>

            <symbol id="markerCR" viewBox="-6.5 -6.5 13 13">
                <circle class="markerCR" r="3" />
            </symbol>
            <symbol id="markerMA" viewBox="-6.5 -6.5 13 13">
                <line class="markerMA" x1="-3" y1="-3" x2="3" y2="3" />
                <line class="markerMA" x1="-3" y1="3" x2="3" y2="-3" />
            </symbol>
            <symbol id="markerSQ" viewBox="-6.5 -6.5 13 13">
                <rect class="markerSQ" x="-3" y="-3" width="6" height="6" />
            </symbol>
            <symbol id="markerTR" viewBox="-6.5 -6.5 13 13">
                <polygon class="markerTR" points="''' + triangle_points(2.5, 0.5) + '''" />
            </symbol>
            <symbol id="markerCurrent" viewBox="-6.5 -6.5 13 13">
                <rect class="markerCurrent" x="-2.5" y="-2.5" width="5" height="5" />
            </symbol>

            <symbol id="markerTB" viewBox="-6.5 -6.5 13 13">
                <circle class="markerTB" r="2.5" />
            </symbol>
            <symbol id="markerTW" viewBox="-6.5 -6.5 13 13">
                <circle class="markerTW" r="2.5" />
            </symbol>

            <symbol id="markerSL" viewBox="0 0 1 1">
                <rect class="markerSL" x="0" y="0" width="1" height="1" />
            </symbol>
            <symbol id="markerDD" viewBox="0 0 1 1">
                <rect class="markerDD" x="0" y="0" width="1" height="1" />
            </symbol>

            <marker id="arrHead" orient="auto" refX="1" refY="2"
                    markerWidth="5" markerHeight="5">
                <polygon points="4,2 0,4 1,2 0,0" />
            </marker>
        '''
    }, # }}}

    'eidogo': { # {{{
        'stone_size': 19,
        'arrowhead_fix': 3,
        'board_padding': 5.0,
        'hoshi_size': 4.0,
        'font_size': 12.0,
        'line_width': 3.0,
        'font_family': 'Arial,Helvetica,sans-serif',
        'sgf_styles': '''
            circle.stoneB {
                fill: url(#blackStoneGradient);
            }

            circle.stoneW {
                fill: url(#whiteStoneGradient);
                stroke: #808080;
                stroke-width: 1;
            }

            circle.hoshi {
                fill: #705f36;
            }

            #boardBackground {
                fill: #ddbc6b;
            }

            .grid {
                stroke: #ae9454;
                stroke-width: 1.0;
                stroke-linecap: square;
            }

            .markerCR, .markerMA, .markerSQ, .markerTR {
                stroke-width: 2.0;
                stroke: #ff0000;
                fill-opacity: 0.0;
            }

            .markerMA {
                stroke-linecap: square;
            }

            .markerTR {
                stroke-linejoin: bevel;
            }

            .markerCurrent {
                fill: #ff0000;
            }

            .markerTB {
                fill: black;
            }

            .markerTW {
                fill: white;
            }

            .markerSL {
                fill: #8800ff;
                fill-opacity: 0.3;
            }

            .markerDD {
                fill: #000000;
                fill-opacity: 0.5;
            }

            .markerLB {
                font-family: %(font_family)s;
                font-size: %(font_size)spx;
                font-weight: bold;
                text-anchor: middle;
                fill: #ff0000;
            }

            .markerLB_W_bg, .markerLB_B_bg, .markerLB_None_bg {
                display: none;
            }

            .markerLN, .markerAR {
                stroke: #0000aa;
                stroke-width: %(line_width)s;
                stroke-linecap: round;
            }

            #arrHead {
                fill: #0000aa;
            }

            .markerAR {
                marker-end: url(#arrHead);
            }
        ''',
        'sgf_defs': '''
            <symbol id="hoshi" viewBox="-2 -2 4 4">
                <circle class="hoshi" r="2" />
            </symbol>

            <radialGradient id="blackStoneGradient"
                cx="-3" cy="-3" fx="-3" fy="-3" r="11" gradientUnits="userSpaceOnUse">
                <stop stop-color="#505050" stop-opacity="1.0" offset="0.0" />
                <stop stop-color="#282828" stop-opacity="1.0" offset="0.6" />
                <stop stop-color="#000000" stop-opacity="1.0" offset="1.0" />
            </radialGradient>
            <radialGradient id="whiteStoneGradient"
                cx="-3" cy="-3" fx="-3" fy="-3" r="11" gradientUnits="userSpaceOnUse">
                <stop stop-color="#ffffff" stop-opacity="1.0" offset="0.0" />
                <stop stop-color="#ffffff" stop-opacity="1.0" offset="0.3" />
                <stop stop-color="#d0d0d0" stop-opacity="1.0" offset="1.0" />
            </radialGradient>

            <symbol id="stoneB" viewBox="-9.5 -9.5 19 19">
                <circle class="stoneB" r="9.5" />
            </symbol>
            <symbol id="stoneW" viewBox="-9.5 -9.5 19 19">
                <circle class="stoneW" r="9" />
            </symbol>

            <symbol id="markerCR" viewBox="-9.5 -9.5 19 19">
                <circle class="markerCR" r="4.5" />
            </symbol>
            <symbol id="markerMA" viewBox="-9.5 -9.5 19 19">
                <line class="markerMA" x1="-4" y1="-4" x2="4" y2="4" />
                <line class="markerMA" x1="-4" y1="4" x2="4" y2="-4" />
            </symbol>
            <symbol id="markerSQ" viewBox="-9.5 -9.5 19 19">
                <rect class="markerSQ" x="-4.5" y="-4.5" width="9" height="9" />
            </symbol>
            <symbol id="markerTR" viewBox="-9.5 -9.5 19 19">
                <polygon class="markerTR" points="''' + triangle_points(3.5) + '''" />
            </symbol>
            <symbol id="markerCurrent" viewBox="-9.5 -9.5 19 19">
                <rect class="markerCurrent" x="-3.5" y="-3.5" width="7" height="7" />
            </symbol>

            <symbol id="markerTB" viewBox="-9.5 -9.5 19 19">
                <circle class="markerTB" r="3.5" />
            </symbol>
            <symbol id="markerTW" viewBox="-9.5 -9.5 19 19">
                <circle class="markerTW" r="3.5" />
            </symbol>

            <symbol id="markerSL" viewBox="0 0 1 1">
                <rect class="markerSL" x="0" y="0" width="1" height="1" />
            </symbol>
            <symbol id="markerDD" viewBox="0 0 1 1">
                <rect class="markerDD" x="0" y="0" width="1" height="1" />
            </symbol>

            <marker id="arrHead" orient="auto" refX="1" refY="2"
                    markerWidth="5" markerHeight="5">
                <polygon points="4,2 0,4 1,2 0,0" />
            </marker>
        '''
    }, # }}}

}

HOSHIS = { # {{{
    5: ((2, 2),),
    7: ((3, 3),),
    9: ((2, 2), (6, 2), (4, 4), (2, 6), (6, 6)),
    13: ((3, 3), (9, 3), (6, 6), (3, 9), (9, 9)),
    19: tuple( (x, y) for x in (3, 9, 15) for y in (3, 9, 15) ),
} # }}}

def attr_escape(val):
    special = (
        ('&', 'amp'),
        ('"', 'quot'),
        ('<', 'lt'),
        ('>', 'gt'),
    )
    val = str(val)
    for f, r in special:
        val = val.replace(f, '&'+r+';')
    return val

def keyfix(key):
    if key[0] == '_':
        key = key[1:]
    key = key.replace('_D', '-')
    key = key.replace('_C', ':')
    return key

def svg_tag(tagname, *arg, **kwarg):
    ret = '<%s %s' % (tagname,
            ' '.join( '%s="%s"' % (keyfix(k), attr_escape(v)) for k, v in kwarg.items() ))
    if arg:
        ret += '>' + ''.join(arg) + ('</%s>' % tagname)
    else:
        ret += ' />'
    return ret

def make_svg(board, options, theme):
    line_width = 1.0

    board_width = board.width * options.stone_size + options.board_padding*2
    board_height = board.height * options.stone_size + options.board_padding*2
    board_tags = []
    stone_tags = []
    marker_tags = []

    # Board background
    board_tags.append(svg_tag('rect', _id='boardBackground',
        x1=0, x2=0, width=board_width, height=board_height))

    # Vertical grid lines
    for x in range(board.width):
        cls = 'grid'
        if x in (0, board.width-1):
            cls += ' gridOutside'
        x1 = x2 = options.board_padding + (x+0.5)*options.stone_size
        y1 = options.board_padding + options.stone_size/2.0
        y2 = board_height - options.stone_size/2.0 - options.board_padding
        board_tags.append(svg_tag('line', _class=cls, x1=x1, y1=y1, x2=x2, y2=y2))

    # Horizontal grid lines
    for y in range(board.height):
        cls = 'grid'
        if y in (0, board.height-1):
            cls += ' gridOutside'
        y1 = y2 = options.board_padding + (y+0.5)*options.stone_size
        x1 = options.board_padding + options.stone_size/2.0
        x2 = board_width - options.stone_size/2.0 - options.board_padding
        board_tags.append(svg_tag('line', _class=cls, x1=x1, y1=y1, x2=x2, y2=y2))

    # Hoshi points
    if board.width == board.height and board.width in HOSHIS:
        for point in HOSHIS[board.width]:
            x, y = ( options.board_padding + (p+0.5)*options.stone_size-options.hoshi_size/2.0 for p in point )
            board_tags.append(svg_tag('use', xlink_Chref='#hoshi',
                x=x, y=y, width=options.hoshi_size, height=options.hoshi_size))

    # Stones
    for y in range(board.height):
        for x in range(board.width):
            xp, yp = ( options.board_padding + p*options.stone_size for p in (x, y) )
            if board.cur_stones[x][y] is not None:
                stone_tags.append(svg_tag('use', xlink_Chref='#stone'+board.cur_stones[x][y],
                    x=xp, y=yp, width=options.stone_size, height=options.stone_size))

    # Lines and Arrows
    for prop in ('LN', 'AR'):
        if prop not in board.current_node.data:
            continue
        for item in board.current_node[prop]:
            start, end = board.convert_points(item.split(':'))
            if start == end:
                continue # illegal
            x1, y1, x2, y2 = ( options.board_padding + (p+0.5)*options.stone_size for p in start + end )
            if prop == 'AR':
                # Adjust for the arrowhead which overshots our target point
                length = math.sqrt((x2-x1)**2 + (y2-y1)**2)
                theta = math.asin((y2-y1)/length)
                if x2 <= x1:
                    theta = math.pi - theta
                x2 -= options.line_width*theme['arrowhead_fix']*math.cos(theta)
                y2 -= options.line_width*theme['arrowhead_fix']*math.sin(theta)
            marker_tags.append(svg_tag('line', _class='marker'+prop,
                x1=x1, y1=y1, x2=x2, y2=y2))

    # Labels
    if 'LB' in board.current_node.data:
        for val in board.current_node['LB']:
            points, label = val.split(':', 1)
            x, y = board.convert_points([points])[0]
            xp, yp = ( options.board_padding + (p+0.5)*options.stone_size for p in (x, y) )
            yp += options.font_size/2.5
            cls = 'markerLB markerLB_' + str(board.cur_stones[x][y])
            #cram = max(min((7.0-len(label))/6.0, 1.0), 0.5)
            marker_tags.append(svg_tag('text', attr_escape(label), _class=cls + '_bg',
                x=xp, y=yp, width=options.stone_size, height=options.stone_size))
            marker_tags.append(svg_tag('text', attr_escape(label), _class=cls + '_fg',
                x=xp, y=yp, width=options.stone_size, height=options.stone_size))
                #transform="translate(%s) scale(%s,1)" % (xp, cram)))

    # Markers
    use_markers = ('CR', 'MA', 'SQ', 'TR')
    if options.current_move_marker != 'None':
        use_markers += ('B', 'W')
    use_markers += ('TB', 'TW', 'DD', 'SL')
    for prop in use_markers:
        if prop not in board.current_node.data:
            continue
        points = board.convert_points(board.current_node[prop])
        if prop in ('W', 'B'):
            prop = options.current_move_marker
        for x, y in points:
            xp, yp = ( options.board_padding + p*options.stone_size for p in (x, y) )
            cls = 'marker' + prop + ' ' 'marker' + prop + '_' + str(board.cur_stones[x][y])
            marker_tags.append(svg_tag('use', xlink_Chref='#marker'+prop, _class=cls,
                x=xp, y=yp, width=options.stone_size, height=options.stone_size))
    # TODO: The handling for DD is bugged. It uses only the current node's DD
    # entries (EidoGo does the same.) DD is supposed to be inherited though
    # (though like zero SGF editors support DD let alone do it correctly).

    crop_width = board_width
    crop_height = board_height
    crop_left = crop_top = 0
    if (options.crop or options.crop_whole_tree) and board.left_edge is not None:
        left = max(0, board.left_edge-options.crop_padding)
        top = max(0, board.top_edge-options.crop_padding)
        right = min(board.width-1, board.right_edge+options.crop_padding)
        bottom = min(board.height-1, board.bottom_edge+options.crop_padding)
        crop_width = options.stone_size * (right-left+1)
        crop_height = options.stone_size * (bottom-top+1)
        if left == 0:
            crop_width += options.board_padding
        else:
            crop_left = options.board_padding + options.stone_size * left
        if top == 0:
            crop_height += options.board_padding
        else:
            crop_top = options.board_padding + options.stone_size * top
        if right == board.width-1:
            crop_width += options.board_padding
        if bottom == board.height-1:
            crop_height += options.board_padding

    svg_vars = {
        'width': round(crop_width),
        'height': round(crop_height),
        'crop_left': crop_left,
        'crop_top': crop_top,
        'board': '\n'.join(board_tags),
        'stones': '\n'.join(stone_tags),
        'markers': '\n'.join(marker_tags),
        'font_family': options.font_family,
        'font_size': options.font_size,
        'line_width': options.line_width,
    }
    if options.verbose:
        sys.stderr.write('Dimensions: %dx%d\n' %
                (round(crop_width), round(crop_height)))
    return (SVG_HEAD + (SVG_DEFS % theme) + SVG_BODY + SVG_FOOT) % svg_vars

def main():
    # Parse command line
    parser = optparse.OptionParser('usage: %prog [options] [input SGF file]')

    parser.add_option('-o', '--output-file', default='-',
            help='write output to FILE ("-" for stdout; the default)', metavar='FILE')
    parser.add_option('-d', '--diagram', action='store_true',
            help="produce a Sensei's style ascii diagram instead of SVG")

    parser.add_option('-g', '--game-number', type="int", default=0,
            help="for sgf game collections, use game number GAME", metavar='GAME')
    parser.add_option('-m', '--move-number', type="int", default=0,
            help="jump to move number MOVE in the main variation" +
            " (defaults to 0 - the starting position)", metavar='MOVE')

    parser.add_option('-c', '--crop', action='store_true',
            help="crop to used area of board (good for problems)")
    parser.add_option('-C', '--crop-whole-tree', action='store_true',
            help="crop to used area of board walking the entire tree to determine crop area")
    parser.add_option('-p', '--crop-padding', type="int", default=1,
            help="pad cropped area by SIZE points on each side (default 1)", metavar='SIZE')

    parser.add_option('-t', '--theme', default='default',
            help="Use theme THEME", metavar='THEME')
    parser.add_option('-b', '--board-padding', type="float",
            help="pad the board by PADDING pixels (default 5)", metavar='PADDING')
    parser.add_option('-s', '--stone-size', type="float",
            help="stone size (in pixels - default 19)", metavar='SIZE')
    parser.add_option('-H', '--hoshi-size', type="float",
            help="hoshi size (in pixels - default 4)", metavar='SIZE')
    parser.add_option('-l', '--line-width', type="float",
            help="width of lines and arrows (LN and AR elements; default 3)", metavar='SIZE')
    parser.add_option('-f', '--font-size', type="float",
            help="font size (default 12)", metavar='SIZE')
    parser.add_option('-F', '--font-family',
            help='font family (default Arial,Helvetica,sans-serif)', metavar='SIZE')
    parser.add_option('-M', '--current-move-marker', default='Current',
            type="choice", choices=('Current', 'None', 'CR', 'MA', 'SQ', 'TR'),
            help='how to mark the current move (options: Current, None, CR, MA, SQ, TR)', metavar='MARKER')

    parser.add_option('-v', '--verbose', action='store_true',
            help='print some verbose output (like image dimensions) to stderr')

    options, args = parser.parse_args()

    theme = THEMES[options.theme]
    if options.font_family is None:
        options.font_family = theme['font_family']
    if options.stone_size is None:
        options.stone_size = theme['stone_size']
    if options.board_padding is None:
        options.board_padding = round(theme['board_padding']*options.stone_size/theme['stone_size'])
    if options.hoshi_size is None:
        options.hoshi_size = theme['hoshi_size']*options.stone_size/theme['stone_size']
    if options.font_size is None:
        options.font_size = theme['font_size']*options.stone_size/theme['stone_size']
    if options.line_width is None:
        options.line_width = theme['line_width']*options.stone_size/theme['stone_size']

    # Get SGF data
    if len(args) > 1:
        sys.stderr.write("No more than one input file may be specified (default is stdin).\n")
        parser.print_help()
        sys.exit()
    elif len(args) == 0 or args[0] == '-':
        infh = sys.stdin
    else:
        infh = file(args[0])
    sgf_data = infh.read()
    infh.close()

    # Process SGF file
    sgf_parser = sgflib.SGFParser(sgf_data)
    board = sgfboard.GoBoard()
    game = sgf_parser.parse()[options.game_number]
    cursor = game.cursor()

    if (options.crop_whole_tree):
        board.execute(cursor.node) # Initial setup node
        stack = [0]
        while len(stack):
            while len(stack) and stack[-1] >= len(cursor.children):
                stack.pop()
                if len(stack):
                    cursor.previous()
                    stack[-1] += 1
            while len(stack) and not cursor.atEnd:
                cursor.next(stack[-1])
                stack.append(0)
                board.execute(cursor.node)
        #
        board.reset_board(reset_edges=False)
        cursor.reset()
    #
    board.execute(cursor.node) # Initial setup node
    while not cursor.atEnd and board.move_number < options.move_number:
        cursor.next()
        board.execute(cursor.node)

    # Generate output
    if options.output_file in (None, '-'):
        outfh = sys.stdout
        if options.verbose:
            sys.stderr.write('Writing to stdout...\n')
    else:
        outfh = file(options.output_file, 'w')
        if options.verbose:
            sys.stderr.write('Writing "%s"...\n' % options.output_file)
    if options.diagram:
        outfh.write(str(board))
    else:
        outfh.write(make_svg(board, options, theme))
    outfh.close()
    if options.verbose:
        sys.stderr.write('Done.\n')

if __name__ == '__main__':
    main()
