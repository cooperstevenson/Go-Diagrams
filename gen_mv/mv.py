#! /usr/bin/python
import subprocess

sgf_file= "/home/cstevens/go/doc/sgf/test_cleaned.sgf"
num_of_moves = 164
sgf2gtp_command = "/home/cstevens/go/src/sgf/sgf2gtp/sgf2gtp_single.py"
oakfoam_command = "/home/cstevens/go/src/oakfoam/oakfoam"
oakfoam_config = "/home/cstevens/go/src/oakfoam/config.gtp"
base_out_dir = "/home/cstevens/go/doc/sgf/game_1/"
for i in range(num_of_moves):
    movenum = (i + 1)
    #function create directory
    subprocess.call(['mkdir', base_out_dir + str(movenum)])
    subprocess.call(['mkdir', base_out_dir + str(movenum) + "/comment"])
    subprocess.call(['mkdir', base_out_dir + str(movenum) + "/gtp"])
    subprocess.call(['mkdir', base_out_dir + str(movenum) + "/crit"])
    subprocess.call(['mkdir', base_out_dir + str(movenum) + "/html"])
    subprocess.call(['mkdir', base_out_dir + str(movenum) + "/infl"])
    subprocess.call(['mkdir', base_out_dir + str(movenum) + "/svg"])
    subprocess.call(['mkdir', base_out_dir + str(movenum) + "/terr"])
    subprocess.call(['mkdir', base_out_dir + str(movenum) + "/var"])
    # if I don't Node will freak out
    subprocess.call(['touch', base_out_dir + str(movenum) + "/comment/" + str(movenum) + ".txt"])
    subprocess.call(['mkdir', base_out_dir + str(movenum) + "/var" + "_0"])
    subprocess.call(['mkdir', base_out_dir + str(movenum) + "/var" + "_1"])
#function end create directory

# function sgf2gtp -- all moves
# basically saying, 'cat sgf_file sgf2gtp_single.py -n movenum > movenum.gtp'
# p1 = subprocess.Popen(['cat', sgf_file], stdout=subprocess.PIPE)
# p2 = subprocess.Popen([str(sgf2gtp_command)], stdin=p1.stdout, stdout=subprocess.PIPE)
# output = p2.communicate()[0]
# print "output" + output

#function individual_sgf2gtp
for i in range(num_of_moves):
    movenum = (i + 1)
    # basically saying, 'cat sgf_file sgf2gtp_single.py -n movenum > movenum.gtp'
    p1 = subprocess.Popen(['cat', sgf_file], stdout=subprocess.PIPE)
    p2 = subprocess.Popen([str(sgf2gtp_command), "-n", str(movenum)], stdin=p1.stdout, stdout=subprocess.PIPE)
    output = p2.communicate()[0]
    # with open(base_out_dir + "test.txt", "w+") as myfile:
    with open(base_out_dir + "test.txt", "a") as myfile:
	        myfile.write(output)
    # print "output" + output

#function run_oakfaom
# subprocess.call(['mkdir', base_out_dir + "mv_" + str(movenum) + "/var" + "_1"])
#    print "p1"
#    p1 = subprocess.Popen(['cat', base_out_dir + "test.txt"], stdout=subprocess.PIPE)
#    print "p2"
#    p2 = subprocess.call([str(oakfoam_command), "-c", str(oakfoam_config), "--log", str(base_out_dir) + "game1.log"],stdin=p1.stdout, stdout=subprocess.PIPE)
# oakfoam_command -c /home/cstevens/go/src/oakfoam/config.gtp --log /tmp/mv_8.log < /tmp/test.gtp

# parse the oakfoam output
# p.py name_of_log_file num_of_moves_to_process
# subprocess.call(['mkdir', base_out_dir + "mv_" + str(movenum) + "/crit"])
subprocess.call(['/home/cstevens/go/src/parse_log/p.py', str(base_out_dir) + "game1.log", str(num_of_moves)])
# python ./p.py /home/cstevens/go/doc/game_1/game_1.log 15


# Create Territory Image

# Create Criticality Image
