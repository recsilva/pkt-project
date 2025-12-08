fd = 0;
file_path = "test.txt";
read_buffer[13]:' ';
read_length = 12;

fd = open(file_path,1,0);

message = "Hello World!";
write_length = 12;

fdw = write(fd,message,write_length);

fdc = close(fd);

fd = open(file_path,0,0);

fdr = read(fd,read_buffer,read_length);

fdc = close(fd);

print(read_buffer);