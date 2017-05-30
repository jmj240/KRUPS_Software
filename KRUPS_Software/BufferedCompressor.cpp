 #include "BufferedCompressor.h"

using namespace std;

uint8_t BufferedCompressor::workSpace[WORKSPACESIZE] = {};

BufferedCompressor::BufferedCompressor()
{
	bufferLoc = 0;
	dataLoc = 0;
	decompressedLength = 0;
  isReady = false;
  isEmpty = true;
}

bool BufferedCompressor::isFull()
{
	return isReady;
}

bool BufferedCompressor::isEmpty()
{
	return isEmpty;
}


size_t BufferedCompressor::getLen()
{
	return dataLoc;
}

void BufferedCompressor::checkIfFull()
{
	if(dataLoc > DATA_SIZE * .9)
	{
		isReady = true;
	}
}

void BufferedCompressor::emptyBuffer()
{
  if(bufferLoc != 0)
	{
		//compress contents of buffer into the data array
		unsigned long compressedTo = blz_pack(buffer, data+dataLoc, bufferLoc, BufferedCompressor::workSpace);
		//track total bytes compressed
		decompressedLength += bufferLoc;
		//reset buffer
		bufferLoc = 0;
		//scoot data buffer up by how much was added
		dataLoc += compressedTo;
	}
}

void BufferedCompressor::sink(const uint8_t* input, size_t len)
{
  isEmpty = false; //data is in the compressor
	for(int i = 0; i < len; i++)
	{
		if(bufferLoc == CHUNK_SIZE) //if buffer has been filled compress and move to begining
		{
			//compress contents of buffer into the data array
			unsigned long compressedTo = blz_pack(buffer, data+dataLoc, CHUNK_SIZE, BufferedCompressor::workSpace);
			//reset buffer
			bufferLoc = 0;
			//scoot data buffer up by how much was added
			dataLoc += compressedTo;
			//track total bytes compressed
			decompressedLength += CHUNK_SIZE;
			checkIfFull(); // see if full bit should be set
		}
			// move data into the buffer
			buffer[bufferLoc] = input[i];
			bufferLoc++;
	}
}

unsigned long BufferedCompressor::readOut(uint8_t* a, size_t& len)
{
	//if the buffer is not compress the rest
  emptyBuffer();

	for(int i = 0;  i < dataLoc; i++)
	{
		a[i] = data[i];
	}
	len = dataLoc; //set the len of data copied
  unsigned long ans = decompressedLength; // save the decompressedLength
	isReady = false; //reset full bit
  //reset to initial
  bufferLoc = 0;
  dataLoc = 0;
  decompressedLength = 0;
  isReady = false;
  isEmpty = true;

	return ans;
}

Packet BufferedCompressor::readIntoPacket()
{
  emptyBuffer();
  Packet p = Packet(decompressedLength, data, dataLoc);

  //reset to initial
  bufferLoc = 0;
  dataLoc = 0;
  decompressedLength = 0;
  isReady = false;
  isEmpty = true;

  return p;
}
