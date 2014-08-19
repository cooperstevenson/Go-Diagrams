#! /usr/bin/env python -tt

import sys
import optparse
import sgflib
import sgfboard

# may not need all of sgflib

def main():
    # Parse command line
    parser = optparse.OptionParser('usage: %prog [options] [input SGF file]')

    parser.add_option('-o', '--output-file', default='-',
            help='write output to FILE ("-" for stdout; the default)', metavar='FILE')
    parser.add_option('-g', '--game-number', type="int", default=0,
                        help="for sgf game collections, use game number GAME", metavar='GAME')
    parser.add_option('-m', '--move-number', type="int", default=0,
            help="jump to move number MOVE in the main variation" +
            " (defaults to 0 - the starting position)", metavar='MOVE')
    parser.add_option('-v', '--verbose', action='store_true',
                        help='print some verbose output (like image dimensions) to stderr')
    parser.add_option('-c', '--comment', action='store_true',
                        help='output the move comment')
    parser.add_option('-s', '--stone', action='store_true',
                        help='output the move')
    options, args = parser.parse_args()

# def get_atr(node, atr):
#     try:
#         return node.data[atr].data[0]
#     except KeyError:
#         return None

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
    cursor.children = 0
     
    board.execute(cursor.node) # Initial setup node
    while not cursor.atEnd and board.move_number < options.move_number:
        cursor.next()
        board.execute(cursor.node)
        # print the moves up to the move number
        # for prop in ('B','W'):
        #     if prop in board.current_node.data:
        #         print "Here is the stone:", board.current_node[prop]
        # print 
        if cursor.nodenum == options.move_number:
            print
            print "Move Number: ", options.move_number
            print "Move Stats: nodenum: %s; index: %s; children: %s" % (cursor.nodenum, cursor.index, len(cursor.children))
            print
            # print cursor.node
            if options.stone:
                for prop in ('B','W'):
                    if prop in board.current_node.data:
                        print "Here is the stone:", board.current_node[prop]
            if options.comment: 
                # works, commenting to reduce output
                # print board.current_node['Comment']
                print "---- Begin Mainline Comment ----"
                print board.current_node['C']
                print "---- End Mainline Comment ----"
                print 
            if len(cursor.children)>=2: # we have a variation, 1 child for mainline and 1 for variation (>=2)
                # variation = cursor.gametree.variations[1]
                #for count in variation:
                # cursor.nodenum+=1
                # while not cursor.atEnd and board.move_number < options.move_number:
                #while not cursor.atEnd and board.move_number < cursor.gametree.variations[1]:
                #  cursor.next()
                #  board.execute(cursor.node)

                # while not cursor.atEnd and board.move_number < cursor.gametree.variations[1]:
                cursor.next(1) # move into the variation
                # for prop in ('B','W'):
                    # if prop in board.current_node.data:
                 #   if prop in cursor.node:
                # print "\nSearch for property 'B':"
                # print col[0].propertySearch("B", 1)
                print "Here is the stone:", cursor.node

                # for count in cursor.gametree.variations[2]: # [1] outputs the first variation, [2] second, etc. step through these 
                   # print "---- Begin Stanza ----"
                   #if len(count) == 1: # beats the hell out of me
                   #   print "Here is the stone:", count
                   #if len(count) == 2:
                   #    print "Here is the comment:", count
                   # print count[-1]
                   # print "---- End Stanza ----"
                   # print
    # Generate output
    if options.output_file in (None, '-'):
        outfh = sys.stdout
        if options.verbose:
            sys.stderr.write('Writing to stdout...\n')
    else:
        outfh = file(options.output_file, 'w')
        if options.verbose:
            sys.stderr.write('Writing "%s"...\n' % options.output_file)
    outfh.close()
    if options.verbose:
        sys.stderr.write('Done.\n')

if __name__ == '__main__':
    main()
