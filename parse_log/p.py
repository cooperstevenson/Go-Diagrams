#! /usr/bin/python
from pyparsing import *
import sys

# +/- sign
sign = Optional(oneOf("+ -"))
point = Literal(".")
floatNumber = Regex(r'\d+(\.\d*)?([eE]\d+)?')
quotes_ = Literal('""')
# The double quotes in criticality is tricky: replace them to '# 808080" and split at the space--writing the file with the .join statement later will make them fall into place
quotes_.setParseAction(lambda pr: pr[0].replace('""','# 808080')
        .split( ' ' ))

# quotes_.setParseAction(lambda pr: pr[0].replace('""','#808080'))

#http://stefaanlippens.net/redirect_python_print
# a simple class with a write method for writing files
class WritableObject:
    def __init__(self):
        self.content = []
    def write(self, string):
        self.content.append(string)

# parser for the territory map 
territory_content = Suppress(Literal("showterritory")) + Suppress(LineEnd()) + Suppress(Literal("=")) + Suppress(LineEnd()) + Group(19 * Group((sign + floatNumber ))) + LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber)))+ LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber)))+ LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber)))+ LineEnd() +  Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber)))+ LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd() + Group(19 * Group((sign + floatNumber)))+ LineEnd() + Group(19 * Group((sign + floatNumber))) + LineEnd()

#parser for the criticality map
#criticality_content = Suppress(Literal("showcriticality")) + Suppress(LineEnd()) + Suppress(Literal("=")) + Suppress(LineEnd()) + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(Literal(" ")) + Suppress(LineEnd())

#parser for the criticality map
criticality_content = Suppress(Literal("showcriticality")) + Suppress(LineEnd()) + Suppress(Literal("=")) + Suppress(LineEnd()) + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + LineEnd() + Group(19 * Group((Literal("#") + Word(alphanums) | quotes_))) + Suppress(LineEnd())


#Open the file
read_file = open( sys.argv[1] ).read()
mv_num = sys.argv[2]

#parse input strings

territory_tokens = territory_content.searchString(read_file).asList()
criticality_tokens = criticality_content.searchString(read_file).asList()


territory = territory_tokens[::]
criticality = criticality_tokens[::]

# the entire list 
for index, list in enumerate(criticality):
    #establish output filenum, incremented by move number
    # filename = 'mv_%d_crit.txt'%(criticality.index(list),)
    #filename = '/home/cstevens/go/doc/sgf/game_1/mv_%d'%(criticality.index(list),) + '/crit/mv_%d_crit.txt'%(criticality.index(list),)
    cur_mv = (index + 1)
    if (cur_mv <= mv_num):
        filename = '/home/cstevens/go/doc/sgf/game_1/' + str(cur_mv) + '/crit/' + str(cur_mv) + '.txt'
        print filename
        # the element number inside the list
        for number in list:
           # fp = open(filename, 'w+')
           fp = open(filename, 'a')
           # index pointer scanning across list stream
           for index in number:
               # place spaces after each slice
               join_num = ''.join(index)+' ',
               #strip the last space of the line (will add leading space if we don't
               replaced_num = [s.replace('\n ','\n') for s in join_num]
               # write buffer to file
               fp.write (' '.join(replaced_num))
           fp.close()

# the entire list 
for index, list in enumerate(territory):
    #establish output filenum, incremented by move number
    #for entire sgf
    # filename = 'mv_%d_terr.txt'%(territory.index(list),)
    cur_mv = (index + 1)
    if (cur_mv <= mv_num):
        filename = '/home/cstevens/go/doc/sgf/game_1/' + str(cur_mv) + '/terr/' + str(cur_mv) + '.txt'
        print filename
        # the element number inside the list
        for number in list:
           # fp = open(filename, 'w+')
           fp = open(filename, 'a')
           #save = open(filename, "a+")
           # index pointer scanning across list stream
           for index in number:
               # place spaces after each slice
               join_num = ''.join(index)+' ',
               #strip the last space of the line (will add leading space if we don't
               replaced_num = [s.replace('\n ','\n') for s in join_num]
               # write buffer to file
               fp.write (' '.join(replaced_num))
           fp.close()
