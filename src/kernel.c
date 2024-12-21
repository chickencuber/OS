#define HEAP_SIZE  0x100000   // 1MB heap size
#define true 1
#define false 0

typedef char bool;
typedef unsigned long uint;

char HEAP_START[HEAP_SIZE]; // the heap, its a char because chars are always 1 byte


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

//allocator
typedef struct {
    uint size;    
    bool free;
    void* ptr;
} Chunk;

struct {
    Chunk chunks[HEAP_SIZE];
    uint size;
    Chunk (*remove)(uint);
    void (*insert)(Chunk, uint);
} Chunks = {{}, 0};

Chunk _remove(uint i) {
    if (i >= Chunks.size) {
        error("Invalid index\0");
        Chunk t;
        return t;
    }
    Chunk temp = Chunks.chunks[i];
    for (uint j = i; j < Chunks.size - 1; j++) {
        Chunks.chunks[j] = Chunks.chunks[j + 1]; 
    }
    Chunks.size--;
    return temp;
}

void _insert(Chunk chunk, uint i) {
    if (Chunks.size >= HEAP_SIZE) {
        error("Too Many Chunks\0");
        return;
    }
    for (uint j = Chunks.size; j > i; j--) {
        Chunks.chunks[j] = Chunks.chunks[j - 1]; 
    }
    Chunks.chunks[i] = chunk;
    Chunks.size++;
}


void def_Allocator() {
    Chunks.remove = &_remove; 
    Chunks.insert = &_insert;
    Chunk c = {
        HEAP_SIZE,
        true,
        &HEAP_START,
    };
    Chunks.insert(c, 0);
}

void* malloc(uint size) {
    Chunk* chunk;
    bool init = false;
    uint i = 0;
    for(i = 0; i < Chunks.size; i++) {
        chunk = &Chunks.chunks[i]; 
        if(chunk->free && chunk->size >= size) {
            init = true;
            break;
        }
    }
    if(!init) {
        error("Alloc Error: Out of memory\0");
        return 0;
    }
    if(chunk -> size > size) {
        Chunk c = {
           chunk -> size - size,
           true,
           chunk->ptr + size,
        };
        chunk->size = size;
        Chunks.insert(c, i+1);
    }
    chunk->free = false;
    return chunk->ptr;
}

typedef struct {
    uint index;
    Chunk* chunk;
} ChunkI;

ChunkI get_chunk(void* ptr) {
    uint i = 0;
    bool init = false;
    Chunk* chunk;
    for(i = 0; i < Chunks.size; i++) {
       chunk = &Chunks.chunks[i];
       if(chunk->ptr == ptr) {
           init = true;
           break;
       }
    }
    if(chunk->free || !init) {
        error("Alloc Error: ptr not found\0");
        ChunkI c;
        return c;
    }
    ChunkI c = {
        i,
        chunk,
    };
    return c;
}

void free(void* ptr) {
    ChunkI temp = get_chunk(ptr);
    Chunk* chunk = temp.chunk;
    uint i = temp.index;
    chunk->free = true;
    if(i < Chunks.size - 1) {
        Chunk* next = &Chunks.chunks[i+1];
        if(next->free) {
            chunk->size += next->size;
            Chunks.remove(i + 1);
        }
    }
    if(i > 0) {
        Chunk* pre = &Chunks.chunks[i-1];
        if(pre->free) {
            chunk->size += pre->size;
            chunk->ptr = pre->ptr;
            Chunks.remove(i-1);
        }
    }
}

void* realloc(void* ptr, uint size) {
    ChunkI temp = get_chunk(ptr);
    Chunk* chunk = temp.chunk;
    uint i = temp.index;
    if(size < chunk->size) {
        if(i < Chunks.size - 1) {
            Chunk* next = &Chunks.chunks[i+1];
            if(next->free) {
                next->size += chunk->size - size;
                next->ptr -= chunk->size - size;
                chunk->size = size;
                return ptr;
            } 
        }
        Chunk c = {
            chunk->size - size,
            true,
            chunk->ptr + size,
        };
        Chunks.insert(c, i + 1);
        chunk->size = size;
        return ptr;
    }
    if(size == chunk->size) {
        return ptr;
    }
    if(i < Chunks.size - 1) {
        Chunk* next = &Chunks.chunks[i+1];
        if(next->free && next->size > size - chunk->size) {
            next->size -= size - chunk->size; 
            next->ptr += size - chunk->size;
            chunk->size = size;
            return ptr;
        } else if(next->free && next->size == size - chunk->size) {
            Chunks.remove(i+1);
            chunk->size = size;
            return ptr;
        }
    }
    char* t = malloc(size);
    for(uint i = 0; i < chunk->size; i++) {
       t[i] = ((char*) ptr)[i]; 
    }
    chunk->free = true;
    if(i < Chunks.size - 1) {
        Chunk* next = &Chunks.chunks[i+1];
        if(next->free) {
            chunk->size += next->size;
            Chunks.remove(i + 1);
        }
    }
    if(i > 0) {
        Chunk* pre = &Chunks.chunks[i-1];
        if(pre->free) {
            chunk->size += pre->size;
            chunk->ptr = pre->ptr;
            Chunks.remove(i-1);
        }
    }
    return t;
}

//test
void test() {
    char* c = malloc(sizeof(char) * 13);
    c[0] = 'H';
    c[1] = 'e';
    c[2] = 'l';
    c[3] = 'l';
    c[4] = 'o';
    c[5] = ' ';
    c[6] = 'w';
    c[7] = 'o';
    c[8] = 'r';
    c[9] = 'l';
    c[10] = 'd';
    c[11] = '!';
    c[12] = '\0';
    println(c);
    free(c);
}

extern void main() {
    //initializes defaults
    io.color = 0x0f;
    def_Allocator();
    test();
}

