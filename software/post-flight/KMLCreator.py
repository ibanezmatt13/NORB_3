split_line = []
latitude = ""
longitude = ""
datastring = ""

data_file = open("N:/DATA.txt", "r") # open NORB 2 data file
output_file = open("N:/NORB2.txt", "w") # open output file

for line in data_file:
    split_line = line.split(",") # split line by comma into fields

    latitude = split_line[3]
    longitude = split_line[4]

    datastring = latitude + "," + longitude + "\n"

    output_file.write(datastring)



data_file.close() # close NORB 2 data file
output_file.close()

print ("Done")
    

