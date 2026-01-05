inp=2199023255552
a=bin(inp).replace("0b", "") 
while(len(a)<64):
    a='0'+a
for i in range(8):
    for j in range(8):
        print(a[i*8+j],end=' ')
    print()