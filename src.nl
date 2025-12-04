fd = 0;
file_path = "test.txt";
read_buffer[4]:'0';
read_length = 3;

fd = open(file_path,0,0);
print(fd);

output = read(fd,read_buffer,read_length);
print(output);

