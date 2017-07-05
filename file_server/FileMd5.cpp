#include <FileMd5.h>


int Compute_file_md5(FILE* fd, char *md5_str)
{
	int i;
	int ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;



	// init md5
	MD5_Init(&md5);

	while (1)
	{
		ret = fread(data ,1, READ_DATA_SIZE,fd);
		if (-1 == ret)
		{
			perror("read");
			return -1;
		}

		MD5_Update(&md5, data, ret);

		if (0 == ret || ret < READ_DATA_SIZE)
		{
			break;
		}
	}


	MD5_Final(md5_value,&md5);

	for(i = 0; i < MD5_SIZE; i++)
	{
		snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);
	}
	md5_str[MD5_STR_LEN] = '\0'; // add end

	return 0;
}
