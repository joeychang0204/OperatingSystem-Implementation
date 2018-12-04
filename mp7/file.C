/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File() {
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
    Console::puts("In file constructor.\n");
    file_size=0;
    file_id=0;
    first_block = 0;
    cur_block=0;
    cur_location=0;
    blocks=NULL;
    inode_block_num = 0;
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) {
    Console::puts("reading from file\n");
    int i = 0;
    
    bool eof = false;
    for(i = 0 ; i < _n ; i++){
		_buf[i] = data[cur_location+i];
	}
    cur_location += i;
	return i;
}


void File::Write(unsigned int _n, const char * _buf) {
    Console::puts("writing to file\n");
    unsigned int count=_n;//initialize count
    while (BLOCKSIZE-HEADER_SIZE<=count){
        if (EoF()){
            unsigned int new_blocks=FILE_SYSTEM->AllocateBlock(0);
            unsigned int* new_block_list= (unsigned int*)new unsigned int[file_size+1];
            for (int i=0;i<file_size;i++)//copy old list
                new_block_list[i]=blocks[i];
            if (blocks!=NULL)
                new_block_list[file_size]=new_blocks;//set new index to new block number
            else
                new_block_list[0]=new_blocks;
            file_size += 1;//increment file size
            delete blocks; //delete old array
            blocks=new_block_list;//set pointer to new array
        }   
        memcpy((void*)(buffer+HEADER_SIZE),_buf,(BLOCKSIZE-HEADER_SIZE));//copy from user buffer to file buffer
        FILE_SYSTEM->disk->write(blocks[cur_block],(unsigned char*)buffer);
        count-=(BLOCKSIZE-HEADER_SIZE);
    }


    int i = 0;
    for(i = 0 ; i < _n ; i++){
        if(EoF())
            file_size += (_n - i);
		data[cur_location+i] = _buf[i];
	}
    cur_location += i;
}

void File::Reset() {
    Console::puts("reset current position in file\n");
    cur_location=0;
    cur_block=0;
    
}

void File::Rewrite() {
    Console::puts("erase content of file\n");
    for(int i = 0; i <= cur_block + 1; i++)
        FILE_SYSTEM->DeallocateBlock(blocks[i]);
    //cur_block=first_block;
    cur_location=0;
    blocks=NULL;
    file_size=0;
}


bool File::EoF() {
    Console::puts("testing end-of-file condition\n");
    if (blocks==NULL){
        return true;
    }
    if (cur_location>=file_size - 1){
        return true;
    }
    else
        return false;
}
