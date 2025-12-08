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
def int fibonacci(i:int){
    if (i == 0){
        return 0;
    }
    if(i == 1){
        return 1;
    }
    if(i > 1){
        return fibonacci(i - 1) + fibonacci(i - 2);
    }
}

a = 6;
b = 6.9;
c = '4';
d = "420";
e[5]:6;
f[5]:6.9;
g[5]:'4';
h[5]:"420";
i = 1;
j = 2.2;
k = 's';
l = "as";
i = e[1];
k = g[1];
l = h[1];
print("printing normal types");
print("|");
print("V");
print(a);
print(b);
print(c);
print(d);
print("============================================================");
print("printing array of all types");
print("|");
print("V");
print(e);
print(f);
print(g);
print(h);
print("============================================================");
print("printing an ellement of an array of all types");
print("|");
print("V");
print(e[0]);
print(f[0]);
print(g[0]);
print(h[0]);
print("============================================================");
print("printing a vriable with asign value of an array");
print("|");
print("V");

print(fibonacci(40));
