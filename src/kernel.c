#define HEAP_START 0x100000   // Start of heap at 1MB
#define HEAP_SIZE  0x100000   // 1MB heap size
#define true 1
#define false 0

typedef char bool;
typedef unsigned int uint;

//io
static struct {
    int x;
    int y;
    char color;
} io;

extern void print(char* arr) {
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

extern void println(char* arr) {
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

extern void error(char* arr) {
    char color = io.color;
    io.color = 0x0C;
    println(arr);
    io.color = color;
}

struct Chunk {
    void* ptr;
    uint size;
    bool free;
    struct Chunk* next; 
    struct Chunk* last;
};

typedef struct Chunk Chunk;

static struct {
    Chunk chunk; 
    uint size; 
    void (*fix_malloc)(Chunk*, uint);
    void (*fix_free)(Chunk*);
} Allocator;

void __fix_malloc__(Chunk* chunk, uint size) {
    if(chunk->size == size) {
        return;
    }
    Chunk c = {
        chunk->ptr + size,
        chunk->size - size,   
        true,
        chunk->next,
        chunk,
    };
    chunk->size = size;
    chunk->next = &c;
    Allocator.size++;
}

void __fix_free__(Chunk* chunk) {
    if(chunk->next) {
        if(chunk->next->free) {
            chunk->size += chunk->next->size;
            if(chunk->next->next) {
                chunk->next->next->last = chunk;
            }
            chunk->next = chunk->next->next;
            Allocator.size--;
        } 
    }
    if(chunk->last) { 
        if(chunk->last->free) {
            __fix_free__(chunk->last);
        }
    }
}

void def_Allocator() {
    Chunk chunk = {
        (void*) HEAP_START,
        HEAP_SIZE,
        true,
    };
    Allocator.chunk = chunk;
    Allocator.size = 1;
    Allocator.fix_malloc = &__fix_malloc__;
    Allocator.fix_free = &__fix_free__;
}

extern void* malloc(uint size) {
    bool init = false;
    void* ptr;
    Chunk* chunk = &Allocator.chunk;
    for(uint i = 0; i < Allocator.size; i++) {
        if(chunk->free && chunk->size >= size) {
            chunk->free = false;
            ptr = chunk->ptr;
            init = true;
            break;
        }
        chunk = chunk->next;
    }
    if(!init) {
        error("Alloc Error: out of memory\0");          
        return false;
    }
    Allocator.fix_malloc(chunk, size);
    return ptr;
}

extern void free(void* ptr) {
    bool init = false;
    Chunk* chunk = &Allocator.chunk;
    for(uint i = 0; i < Allocator.size; i++) {
        if(chunk->ptr == ptr) {
            if(chunk->free) {
                error("Alloc Error: ptr not found\0");
                return;
            }
            init = true;
            break;
        }
        chunk = chunk->next;
    }
    if(!init) {
        error("Alloc Error: ptr not found\0");          
        return;
    }
    chunk->free = true;
    Allocator.fix_free(chunk);
}

void test() {
    char* c = malloc(1);    
    print("Hello World!\0");
}

extern void main() {
    //initializes defaults
    def_Allocator();
    io.color = 0x0f;
    test();
}
