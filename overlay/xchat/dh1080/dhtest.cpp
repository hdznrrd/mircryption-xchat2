 



#include "dh1080.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
	DH1080 dh;
	cout << "my pub key: " << dh.getNewPublicKey() << endl;
	
	if(argc>1)
	cout << "shared secret: " << dh.computeSymetricKey(argv[1]) << endl;

	dh.flush();
	return 0;
}

