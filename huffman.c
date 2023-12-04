#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX_FILE_NAME 100
#define MAX_CHARACTERS 256
#define MAX_BUFFER_SIZE 256
#define INVALID_BIT_READ -1
#define INVALID_BIT_WRITE -1
#define FAILURE 1
#define SUCCESS 0
#define FILE_OPEN_FAIL -1
#define END_OF_FILE -1
#define MEM_ALLOC_FAIL -1

// Global Variables
int num_alphabets = 256;
int num_active = 0;
int num_nodes = 0;
int free_index = 1;
int stack_top;
int bits_in_buffer = 0;
int current_bit = 0;
int eof_input = 0;

int *frequency = NULL;
int *leaf_index = NULL;
int *parent_index = NULL;
int *stack;

unsigned int original_size = 0;

unsigned char buffer[MAX_BUFFER_SIZE];

typedef struct {
    char *text;
    int size;
    int *counts;
} ThreadData;

typedef struct {
    int index;
    unsigned int weight;
} node_t;

node_t *nodes = NULL;

// Function Prototypes
void print_help (int issue);
int checkArgs (int argc, char **argv);
double timestamp ();
void init ();
void finalise ();
int encode (const char* ifile, const char *ofile);
void determine_size (FILE *f);
void allocate_tree ();
int add_node (int index, int weight);
void add_leaves ();
int write_header (FILE *f);
void build_tree ();
void encode_alphabet (FILE *fout, int character);
int write_bit (FILE *f, int bit);
int flush_buffer (FILE *f);
int decode (const char* ifile, const char *ofile);
int read_header (FILE *f);
void decode_bit_stream (FILE *fin, FILE *fout);
int read_bit (FILE *f);

// Prints helpful messages to console if parameters are wrong
void print_help (int issue) 
{
    switch(issue)
    {
        case 1:
            printf("Program requires 5 parameters\n");
            printf("\nUSAGE: ./huffman [encode | decode] <input file> <output file> <Thread Count>\n\n");
            break;
        case 2:
            printf("Parameter one needs to be 'encode' OR 'decode'\n");
            printf("\nUSAGE: ./huffman [encode | decode] <input file> <output file> <Thread Count>\n\n");
            break;
        case 3:
            printf("Parameter two and three need to be longer than 4 characters\n");
            printf("\nUSAGE: ./huffman [encode | decode] <input file> <output file> <Thread Count>\n\n");
            break;
        case 4:
            printf("For encoding parameter two should end in \".txt\" and three should end in \".dat\"\n");
            printf("\nUSAGE: ./huffman [encode | decode] <input file> <output file> <Thread Count>\n\n");
            break;
        case 5:
            printf("For decoding parameter two should end in \".dat\" and three should end in \".txt\"\n");
            printf("\nUSAGE: ./huffman [encode | decode] <input file> <output file> <Thread Count>\n\n");
            break;
        default:
            printf("\nSomething went wrong, please try again.\n");
            break;
    }
}

// Parses argv for issues
int checkArgs (int argc, char **argv)
{
    if (argc != 5)
    {
        print_help (1);
        return FAILURE;
    }

    if (strcmp (argv[1], "encode"))
    {
        if (strcmp (argv[1], "decode"))
        {
            print_help (2);
            return FAILURE;
        }
    }

    if (strlen (argv[2]) <= 4 || strlen (argv[3]) <= 4)
    {
        print_help (3);
        return FAILURE;
    }

    if (!strcmp (argv[1], "encode") && (strcmp (strrchr (argv[2], '\0') - 4, ".txt") || strcmp (strrchr (argv[3], '\0') - 4, ".dat")))
    {
        print_help (4);
        return FAILURE;
    }

    if (!strcmp (argv[1], "decode") && (strcmp (strrchr (argv[2], '\0') - 4, ".dat") || strcmp (strrchr (argv[3], '\0') - 4, ".txt")))
    {
        print_help (5);
        return FAILURE;
    }

    return SUCCESS;
}

// Tracks time of code execution
double timestamp ()
{
    struct timeval time;
    
    gettimeofday (&time, NULL);
    return (time.tv_sec + (time.tv_usec / 1000000.0));
}

// initialize variable size
void init () 
{
    frequency = (int *)calloc(2 * num_alphabets, sizeof(int));
    leaf_index = frequency + num_alphabets - 1;
}

// free memory
void finalise () 
{
    free(parent_index);
    free(frequency);
    free(nodes);
}

// Controls the encoding phase
int encode (const char* ifile, const char *ofile) 
{
    FILE *fin, *fout;
    
    if ((fin = fopen(ifile, "rb")) == NULL) 
    {
        perror("Failed to open input file");

        return FILE_OPEN_FAIL;
    }
    
    if ((fout = fopen(ofile, "wb")) == NULL) 
    {
        perror("Failed to open output file");
        fclose(fin);

        return FILE_OPEN_FAIL;
    }

    determine_size(fin);
    stack = (int *) calloc(num_active - 1, sizeof(int));
    allocate_tree();

    add_leaves();
    write_header(fout);
    build_tree();
    fseek(fin, 0, SEEK_SET);
    int c;

    while ((c = fgetc(fin)) != EOF)
    {
        encode_alphabet(fout, c);
    }

    flush_buffer(fout);
    free(stack);
    fclose(fin);
    fclose(fout);

    return 0;
}

// Finds size of input file and number of unique characters in file
void determine_size (FILE *f) 
{
    fseek(f, 0, SEEK_SET);
    fseek (f, 0, SEEK_END);
    original_size = ftell (f);
    fseek(f, 0, SEEK_SET);

    for (int c = 0; c < num_alphabets; ++c)
    {
        if (frequency && frequency[c] > 0)
        {
            ++num_active;
        }
    }
}

// Allocates memory
void allocate_tree () 
{
    nodes = (node_t *)calloc(2 * num_active, sizeof(node_t));
    parent_index = (int *)calloc(num_active, sizeof(int));
}

// Helps build huffman tree
int add_node (int index, int weight) 
{
    int i = num_nodes++;

    while (i > 0 && nodes[i].weight > weight) 
    {
        memcpy(&nodes[i + 1], &nodes[i], sizeof(node_t));

        if (nodes[i].index < 0)
        {
            ++leaf_index[-nodes[i].index];
        }
        else
        {
            ++parent_index[nodes[i].index];
        }

        --i;
    }

    ++i;
    nodes[i].index = index;
    nodes[i].weight = weight;

    if (index < 0)
    {
        leaf_index[-index] = i;
    }
    else
    {
        parent_index[index] = i;
    }

    return i;
}

// Helps build huffman tree
void add_leaves () 
{
    int i, freq;
    
    for (i = 0; i < num_alphabets; ++i) 
    {
        freq = frequency[i];
        
        if (freq > 0)
        {
            add_node (-(i + 1), freq);
        }
    }
}

// Writes huffman tree to output file as header
int write_header (FILE *f) 
{
     int i, j, byte = 0, size = sizeof(unsigned int) + 1 + num_active * (1 + sizeof(int));
     unsigned int weight;
     char *buffer = (char *) calloc(size, 1);

     if (buffer == NULL)
     {
        return MEM_ALLOC_FAIL;
     }
     
     j = sizeof(int);

     while (j--)
     {
        buffer[byte++] = (original_size >> (j << 3)) & 0xff;
     }
     
     buffer[byte++] = (char) num_active;

     for (i = 1; i <= num_active; ++i) 
     {
        weight = nodes[i].weight;
        buffer[byte++] = (char) (-nodes[i].index - 1);
        j = sizeof(int);
        
        while (j--)
        {
            buffer[byte++] = (weight >> (j << 3)) & 0xff;
        }
     }

     fwrite(buffer, 1, size, f);
     free(buffer);
     
     return 0;
}

// Helps build huffman tree
void build_tree () 
{
    int a, b, index;
    
    while (free_index < num_nodes) 
    {
        a = free_index++;
        b = free_index++;
        index = add_node (b/2, nodes[a].weight + nodes[b].weight);
        parent_index[b/2] = index;
    }
}

// Creates huffman code for character
void encode_alphabet (FILE *fout, int character) 
{
    int node_index;
    stack_top = 0;
    node_index = leaf_index[character + 1];
    
    while (node_index < num_nodes) 
    {
        stack[stack_top++] = node_index % 2;
        node_index = parent_index[(node_index + 1) / 2];
    }
    
    while (--stack_top > -1)
    {
        write_bit(fout, stack[stack_top]);
    }
}

// Writes to buffer
int write_bit (FILE *f, int bit) 
{
    if (bits_in_buffer == MAX_BUFFER_SIZE << 3) 
    {
        size_t bytes_written = fwrite(buffer, 1, MAX_BUFFER_SIZE, f);
        
        if (bytes_written < MAX_BUFFER_SIZE && ferror(f))
        {
            return INVALID_BIT_WRITE;
        }

        bits_in_buffer = 0;
        memset(buffer, 0, MAX_BUFFER_SIZE);
    }

    if (bit)
    {
        buffer[bits_in_buffer >> 3] |= (0x1 << (7 - bits_in_buffer % 8));
    }

    ++bits_in_buffer;
    
    return SUCCESS;
}

// Flush buffer
int flush_buffer (FILE *f) 
{
    if (bits_in_buffer) 
    {
        size_t bytes_written = fwrite(buffer, 1, (bits_in_buffer + 7) >> 3, f);
        
        if (bytes_written < MAX_BUFFER_SIZE && ferror(f))
        {
            return -1;
        }

        bits_in_buffer = 0;
    }

    return 0;
}

// Controls the decoding phase
int decode (const char* ifile, const char *ofile) 
{
    FILE *fin, *fout;
    
    if ((fin = fopen(ifile, "rb")) == NULL) 
    {
        perror("Failed to open input file");

        return FILE_OPEN_FAIL;
    }
    
    if ((fout = fopen(ofile, "wb")) == NULL) 
    {
        perror("Failed to open output file");
        fclose(fin);

        return FILE_OPEN_FAIL;
    }

    if (read_header(fin) == 0) 
    {
        build_tree();
        decode_bit_stream(fin, fout);
    }
    
    fclose(fin);
    fclose(fout);

    return 0;
}

// Read the huffman tree from header
int read_header (FILE *f) 
{
    int i, j, byte = 0, size;
    size_t bytes_read;
    unsigned char buff[4];

    bytes_read = fread(&buff, 1, sizeof(int), f);
    
    if (bytes_read < 1)
    {
    return END_OF_FILE;
    }
    
    byte = 0;
    original_size = buff[byte++];

    while (byte < sizeof(int))
    {
    original_size = (original_size << (1 << 3)) | buff[byte++];
    }
    
    bytes_read = fread(&num_active, 1, 1, f);

    if (bytes_read < 1)
    {
    return END_OF_FILE;
    }
    
    allocate_tree();

    size = num_active * (1 + sizeof(int));
    unsigned int weight;
    char *buffer = (char *)calloc(size, 1);

    if (buffer == NULL)
    {
    return MEM_ALLOC_FAIL;
    }
    
    fread(buffer, 1, size, f);
    byte = 0;

    for (i = 1; i <= num_active; ++i) 
    {
        nodes[i].index = -(buffer[byte++] + 1);
        j = 0;
        weight = (unsigned char) buffer[byte++];

        while (++j < sizeof(int)) 
        {
            weight = (weight << (1 << 3)) | (unsigned char) buffer[byte++];
        }

        nodes[i].weight = weight;
    }

    num_nodes = (int) num_active;
    free(buffer);
    
    return 0;
}

// decodes input file to output file
void decode_bit_stream (FILE *fin, FILE *fout) 
{
    int i = 0, bit, node_index = nodes[num_nodes].index;
    
    while (1) 
    {
        bit = read_bit(fin);
        
        if (bit == -1)
        {
            break;
        }

        node_index = nodes[node_index * 2 - bit].index;
        
        if (node_index < 0) 
        {
            char c = -node_index - 1;
            fwrite(&c, 1, 1, fout);
            
            if (++i == original_size)
            {
                break;
            }

            node_index = nodes[num_nodes].index;
        }
    }
}

// decodes each bit from input file
int read_bit (FILE *f) 
{
    if (current_bit == bits_in_buffer) 
    {
        if (eof_input)
        {
            return END_OF_FILE;
        } 
        else 
        {
            size_t bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE, f);

            if (bytes_read < MAX_BUFFER_SIZE) 
            {
                if (feof(f))
                {
                    eof_input = 1;
                }
            }

            bits_in_buffer = bytes_read << 3;
            current_bit = 0;
        }
    }

    if (bits_in_buffer == 0)
    {
        return END_OF_FILE;
    }
        
    int bit = (buffer[current_bit >> 3] >> (7 - current_bit % 8)) & 0x1;
    ++current_bit;

    return bit;
}

// counts the frequency of each character
void* count_characters (void *thread_data)
{
    ThreadData *data = (ThreadData*)thread_data;

    for (int i = 0; i < data->size; i++) {
        char current_char = data->text[i];
        data->counts[current_char]++;
    }

    pthread_exit(NULL);
}

// Divides file among threads for counting
void pthread_char_frequency (const char *file_name, int num_threads, int *frequency)
{
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", file_name);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = (char *)malloc(file_size);
    fread(file_content, 1, file_size, file);
    fclose(file);

    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    ThreadData *thread_data = (ThreadData *)malloc(num_threads * sizeof(ThreadData));
    
    int chunk_size = file_size / num_threads;
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].text = file_content + i * chunk_size;
        thread_data[i].size = (i == num_threads - 1) ? (file_size - i * chunk_size) : chunk_size;
        thread_data[i].counts = (int *)calloc(MAX_CHARACTERS, sizeof(int));

        pthread_create(&threads[i], NULL, count_characters, (void*)&thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < num_threads; i++) {
        for (int j = 0; j < MAX_CHARACTERS; j++) {
            frequency[j] += thread_data[i].counts[j];
        }
        free(thread_data[i].counts);
    }

    free(file_content);
    free(threads);
    free(thread_data);
}

int main (int argc, char **argv)
{
    // check parameters
    if (checkArgs (argc, argv)) { return FAILURE; }

    // begin timer
    double begin = timestamp ();

    // name variables
    char *code = argv[1];
    char *in = argv[2];
    char *out = argv[3];
    int num_threads = atoi (argv[4]);

    init();

    if (strcmp(code, "encode") == 0)
    {
        pthread_char_frequency(in, num_threads, frequency);
        encode(in, out);
    }
    else if (strcmp(code, "decode") == 0)
    {
        decode(in, out);
    }

    finalise();

    double end = timestamp ();
    
    printf ("TOTAL TIME %5.2lf\n", (end - begin));
    
    return SUCCESS;
}
