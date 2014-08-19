# Implements a very rudimentary board class
import sgflib, re

class GoBoard(object):
    # This is pretty primitive

    width = None
    height = None

    cur_stones = None
    cur_DD = None
    cur_VW = None
    move_numbers = None
    current_node = None

    black_prisoners = 0
    white_prisoners = 0
    move_number = 0
    to_play = 'B'

    # For shrink-wrapping, determine leftmost, rightmost, etc. used points
    left_edge = None
    right_edge = None
    top_edge = None
    bottom_edge = None

    _adjacent = ((-1, 0), (1, 0), (0, -1), (0, 1))

    def __init__(self, width=19, height=None):
        if height is None:
            height = width
        self.width = width
        self.height = height
        self.move_number = 0
        self.reset_board()

    def _make_empty_board(self, width, height, val=None):
        ret = []
        for x in range(width):
            ret.append([val] * height)
        return ret

    def reset_board(self, reset_edges=True):
        self.cur_stones = self._make_empty_board(self.width, self.height)
        self.cur_DD = self._make_empty_board(self.width, self.height, False)
        self.cur_VW = self._make_empty_board(self.width, self.height, False)
        self.move_numbers = self._make_empty_board(self.width, self.height)
        self.black_prisoners = 0
        self.white_prisoners = 0
        self.move_number = 0
        if reset_edges:
            self.left_edge = self.right_edge = self.top_edge = self.bottom_edge = None

    def execute(self, node):
        self.current_node = node

        if 'SZ' in node.data:
            sizes = node['SZ'][0].split(':')
            if len(sizes) == 1:
                self.width = self.height = int(sizes[0])
            elif len(sizes) == 2:
                self.width, self.height = [ int(x) for x in sizes ]
            else:
                raise ValueError('Board size must be 1 or 2 values (%s)' % repr(node['SZ'][0]))
            self.reset_board(reset_edges=False)

        for prop, kind in (('AB', 'B'), ('AW', 'W'), ('AE', None)):
            if prop in node.data:
                self.add_stones(kind, node[prop])

        for prop in ('B', 'W'):
            if prop in node.data:
                self.do_move(prop, node[prop])

        mpoints = []
        if 'LB' in node.data:
            mpoints.extend([ v.split(':', 1)[0] for v in node['LB'] ])

        for prop in ('CR', 'MA', 'SQ', 'TR', 'DD', 'SL', 'LN', 'AR'):
            if prop not in node.data:
                continue
            mpoints.extend(node[prop])

        for x, y in self.convert_points(mpoints):
            self._crop_point(x, y)

        if 'PL' in node.data:
            self.to_play = node['PL'][0]

    def _crop_point(self, x, y):
        if self.left_edge is None or x < self.left_edge:
            self.left_edge = x

        if self.right_edge is None or x > self.right_edge:
            self.right_edge = x

        if self.top_edge is None or y < self.top_edge:
            self.top_edge = y

        if self.bottom_edge is None or y > self.bottom_edge:
            self.bottom_edge = y

    def add_stones(self, color, points):
        for x, y in self.convert_points(points):
            self._crop_point(x, y)
            if not self._valid_point(x, y):
                continue
            self.cur_stones[x][y] = color

    def do_move(self, color, points):
        self.move_number += 1
        if color == 'B':
            self.to_play = 'W'
        else:
            self.to_play = 'B'

        if len(points) > 1:
            raise ValueError('A move must specify exactly one point.')
        if len(points) < 1:
            return # a pass possibly?
        if points[0] == '' or points[0] == 'tt' and self.width <= 19 and self.height <= 19:
            return # a pass

        x, y = self.convert_points(points)[0]
        if not self._valid_point(x, y):
            raise ValueError('Invalid move point specified');

        self.cur_stones[x][y] = color
        self._crop_point(x, y)
        if self.move_numbers[x][y] is None:
            self.move_numbers[x][y] = []
        self.move_numbers[x][y].append(self.move_number)

        for a, b in self._adjacent:
            if not self._valid_point(x+a, y+b):
                continue
            if self.cur_stones[x+a][y+b] == self.to_play:
                self._check_capture(x+a, y+b)

        self._check_capture(x, y)

    def _chain(self, x, y):
        chain = set()
        queue = [(x, y)]
        color = self.cur_stones[x][y]
        while len(queue):
            next = queue.pop()
            if not self._valid_point(*next) or next in chain:
                continue
            if self.cur_stones[next[0]][next[1]] == color:
                chain.add(next)
                queue.extend([ (next[0]+a, next[1]+b) for a, b in self._adjacent ])
        return chain

    def _liberties(self, chain):
        liberties = set()
        for x, y in chain:
            for a, b in self._adjacent:
                if self._valid_point(x+a, y+b) and self.cur_stones[x+a][y+b] is None:
                    liberties.add((x+a, y+b))
        return liberties

    def _check_capture(self, x, y):
        chain = self._chain(x, y)
        liberties = self._liberties(chain)
        if len(liberties):
            return
        for x, y in chain:
            self.cur_stones[x][y] = None

    def convert_points(self, points):
        def cp(v):
            if v >= 'a' and v <= 'z':
                return ord(v)-ord('a')
            else:
                return ord(v)-ord('A')

        ret = []
        for item in points:
            if re.match(r'^[a-zA-Z]{2}$', item):
                ret.append(tuple( cp(x) for x in item ))
            elif re.match(r'^[a-zA-Z]{2}:[a-zA-Z]{2}$', item):
                x1, y1, x2, y2 = cp(item[0]), cp(item[1]), cp(item[3]), cp(item[4])
                if x1 > x2:
                    x1, x2 = x2, x1
                if y1 > y2:
                    y1, y2 = y2, y1
                for y in range(y1, y2+1):
                    for x in range(x1, x2+1):
                        ret.append((x, y))
            elif item == '':
                pass # a pass!
            else:
                raise ValueError('Invalid point list (%s)' % repr(item))
        return ret

    def _diag(self, x, y):
        val = '.'
        if self.cur_stones[x][y] == 'B':
            val = 'X'
        elif self.cur_stones[x][y] == 'W':
            val = 'O'

        if val == '.':
            if self.width == self.height == 19:
                if x in (3, 9, 15) and y in (3, 9, 15):
                    val = ','
            elif self.width == self.height == 13:
                if x in (3, 9) and y in (3, 9) or x == y == 6:
                    val = ','
            elif self.width == self.height == 9:
                if x in (2, 6) and y in (2, 6) or x == y == 4:
                    val = ','
            elif self.width == self.height and self.width % 2:
                if x == y == (self.width-1)/2:
                    val = ','

        return val

    def _valid_point(self, x, y):
        if x < 0 or y < 0 or x >= self.width or y >= self.height:
            return False
        return True

    def __str__(self):
        ret = '$$' + self.to_play + ' Comment\n'
        ret += '$$  ' + '-' * (self.width*2+1) + '\n'
        ret += ''.join([
                '$$ | ' + ' '.join([ self._diag(x, y) for x in range(self.width) ]) + ' |\n'
                for y in range(self.height) ])
        ret += '$$  ' + '-' * (self.width*2+1) + '\n'

        return ret

# Hi
