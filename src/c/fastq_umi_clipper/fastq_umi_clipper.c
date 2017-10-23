/*
Copyright 2016 National Center for Genome Resources (NCGR)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the NCGR nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL NCGR BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bstrlib.h>

// Could go in a header file
struct fastq_block 
{
  bstring header1;
  bstring sequence;
  bstring header2;
  bstring qualities;
};

int main (int argc, char *argv[]) 
{
  if (argc < 3)
  {
    fprintf (stderr, "\nUsage: fastq_umi_clipper fastqfilename clip_length g_length trim_length\n\n") ;
    fprintf (stderr, "   fastqfilename: path to a fastq file\n") ;
    fprintf (stderr, "   clip_length: number of nucleotides to move from the start of the read and put in header 1\n") ;
    fprintf (stderr, "   g_length: number of nucleotides (Gs) to delete after the UMI\n") ;
    fprintf (stderr, "   trim_length: number of chars to remove from the end of the header before appending the UMI\n") ;
    fprintf (stderr, "\nWarning: not much arg checking is done so unexpected behavior may be encountered. For example if you enter a clip_length longer than the read length, or negative values. Output files will be overwritten without warning. \n\n");
    exit (-1);
  }

  char *fastq_file = argv[1];
  int clip_length = atoi(argv[2]);
  int g_length = atoi(argv[3]);
  int trim_length = atoi(argv[4]);

  // a struct is not really needed but you might want to send it off to some function
  struct fastq_block block;
    
  bstring line = bfromcstr(""); //must be initialised for bsreadln()
  int block_line_counter = 0;
  int total_counter = 0; 
  int good_count = 0;
  int bad_count = 0;
    
  
  // how to make a file stream
  //char *pointer_name = "/path/to/file";
  //FILE *stream = fopen (pointer_name, "w");
  
  FILE *in  = fopen (fastq_file, "r"); 
 
  if (in == NULL) 
  {
    perror("problem opening file argv[1]");
  }  
 
  //Use the following block if you want a bstream instead of just 
  //reading from the filehandle  
  //struct bStream *fastq_stream = bsopen ((bNread)&fread, in);  
  //if (fastq_stream == NULL)
  //{
  //  perror("problem opening bStream fastq_stream");
  //}
  //while (bsreadln (line, fastq_stream, (char) '\n') == 0)

  
  // Comment out this while line if you want to use a bstream one above 
  while((line = bgets ((bNgetc) fgetc, in, (char) '\n')) != NULL)
  {
     btrimws (line);
     //printf("%s\n", line->data); 
        
     block_line_counter++;
     total_counter++;
     
     // if first line assign block.header1
     if(block_line_counter == 1)
     {
        block.header1 = bstrcpy(line);
        //printf("%s\n", line->data);
     }     
     // if second line assign block.sequence
     else if(block_line_counter == 2)
     {
        block.sequence = bstrcpy(line);
        //printf("%s\n", line->data);
     }     
     // if third line assign block.header2
     else if(block_line_counter == 3)
     {
        block.header2 = bstrcpy(line);
        //printf("%s\n", line->data);
     }
     // if fourth line assign block.qualities
     else if(block_line_counter == 4)
     {
        block.qualities = bstrcpy(line);
        //printf("%s\n", line->data);
               
        // You now have a four line block in the struct
        
        // remove the index from the header1
        int header1_length = block.header1->slen;
        int trim_start = header1_length-trim_length;
        
        bdelete (block.header1 , trim_start , trim_length);
                
        // get the clipped UMI string off the sequence string
        bstring umi = bmidstr (block.sequence , 0 , clip_length);
         
        // cut off chars at the start of sequence and qualities
        bdelete (block.sequence , 0 , clip_length+g_length);
        bdelete (block.qualities , 0 , clip_length+g_length);
        
        // append the UMI to the header1
        bstring s = bfromcstr("#");
        bconcat(block.header1, s);        
        bconcat(block.header1, umi);        

        // temporarily put the index in header2
        //bconcat(block.header2, index);
        
        // do some output. 
        printf("%s\n%s\n%s\n%s\n", block.header1->data,block.sequence->data,block.header2->data,block.qualities->data);
           
        // reset      
        block_line_counter = 0;       
        bdestroy(block.header1);
        bdestroy(block.sequence);
        bdestroy(block.header2);
        bdestroy(block.qualities);      
        bdestroy(s);
        bdestroy(umi);
     }     
     
     if( ! (total_counter % 1000000) )
     {
        fprintf (stderr, "Processed: %d records\n", total_counter);
     }
     
     //line = bfromcstr(""); // if using bsreadln
     bdestroy (line); // if using bgets
  }
  
  // a bit of clean up
  fclose(in);
  
  return 0;
}



