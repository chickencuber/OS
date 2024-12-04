static struct {
    int x;
    int y;
    char color;
} io;

void print(char* arr) {
    int i = 0;
    char* mem = (char*)0xb8000;
    while (arr[i] != 0) {
        if(arr[i] == '\n') {
            io.y++;
            io.x = 0;
            i++;
            continue;
        }
        mem[io.x + 160 * io.y] = arr[i];
        mem[io.x+1 + 160 * io.y] = io.color;
        io.x+=2; 
        i++;
    }
}

void println(char* arr) {
    int i = 0;
    char* mem = (char*)0xb8000;
    while (arr[i] != 0) {
        if(arr[i] == '\n') {
            io.y++;
            io.x = 0;
            i++;
            continue;
        }
        mem[io.x + 160 * io.y] = arr[i];
        mem[io.x+1 + 160 * io.y] = io.color;
        io.x+=2; 
        i++;
    }
    io.x = 0;
    io.y++;
}

extern void main() {
    io.color = 0x0f;
    println("Hello\0");
    println("World!\0");
}
