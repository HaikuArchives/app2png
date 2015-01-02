#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <AppFileInfo.h>
#include <Bitmap.h>
#include <BitmapStream.h>
#include <File.h>
#include <Path.h>
#include <String.h>
#include <TranslatorRoster.h>

#include <fs_attr.h>

void
print_help(const char *firstarg) {
	fprintf(stderr, "Usage: %s [-h, --help] file1 [file2 [file3 [etc...]]]\n",
		firstarg);
	exit(0);
}



int main(int argc, char** argv)
{
	if (argc < 1)
		print_help(argv[0]);
	
	static struct option options[] = {
		{"help", 0, 0, 0},
		{0, 0, 0, 0}
	};
	
	opterr = 0;
	
	while (true) {
		int longindex;
		int index = getopt_long(argc, argv, "h", options, &longindex);
		if (index < 0)
			break;
		if (index == 0)
			print_help(*argv);
		else if(index == 'h')
			print_help(*argv);
	}
	
	if (optind >= argc)
		print_help(*argv);
	
	while (optind < argc) {
		char *fileName = argv[optind++];
		
		BFile file(fileName, B_READ_ONLY);
		if (file.InitCheck() != B_OK) {
			fprintf(stderr, "Opening file '%s' failed: %s\n",
				fileName, strerror(file.InitCheck()));
			exit(1);
		}
		
		BPath filePath(fileName);
		
		BAppFileInfo appInfo(&file);

		BBitmap mediumBitmap(BRect(0, 0, 15, 15), B_BITMAP_NO_SERVER_LINK,
			B_CMAP8);

		BBitmap largeBitmap(BRect(0, 0, 31, 31), B_BITMAP_NO_SERVER_LINK,
			B_CMAP8);
		
		appInfo.GetIcon(&mediumBitmap, B_MINI_ICON);
		appInfo.GetIcon(&largeBitmap, B_LARGE_ICON);

		BTranslatorRoster *roster = BTranslatorRoster::Default();

		status_t status;

		BBitmap mediumColorBitmap(BRect(0, 0, 15, 15), B_BITMAP_NO_SERVER_LINK,
			B_RGB32);
		BBitmap largeColorBitmap(BRect(0, 0, 31, 31), B_BITMAP_NO_SERVER_LINK,
			B_RGB32);

		mediumColorBitmap.ImportBits(&mediumBitmap);
		largeColorBitmap.ImportBits(&largeBitmap);

		BBitmapStream mediumBitmapStream(&mediumColorBitmap);
		BString mediumFileName;
		mediumFileName.SetToFormat("%s-icon_16.png", filePath.Leaf());
		BFile mediumFile(mediumFileName, B_READ_WRITE | B_CREATE_FILE);
		
		if (mediumFile.InitCheck() != B_OK) {
			fprintf(stderr, "Creating file failed: %s\n",
				strerror(mediumFile.InitCheck()));
			exit(1);
		}
		
		status = roster->Translate(&mediumBitmapStream, NULL, NULL, &mediumFile,
			B_PNG_FORMAT);

		if (status != B_OK) {
			fprintf(stderr, "Error converting medium icon to PNG: %s\n",
				strerror(status));
			exit(1);
		}
		
		fprintf(stderr, "Written medium image to ");
		fprintf(stdout, "%s-icon_16.png\n", filePath.Leaf());

		BBitmap *mediumDummy = NULL;
		mediumBitmapStream.DetachBitmap(&mediumDummy);


		BBitmapStream largeBitmapStream(&largeColorBitmap);
		BString largeFileName;
		largeFileName.SetToFormat("%s-icon_32.png", filePath.Leaf());
		BFile largeFile(largeFileName, B_READ_WRITE | B_CREATE_FILE);
		
		if (largeFile.InitCheck() != B_OK) {
			fprintf(stderr, "Creating file failed: %s\n",
				strerror(largeFile.InitCheck()));
			exit(1);
		}
		
		status = roster->Translate(&largeBitmapStream, NULL, NULL, &largeFile,
			B_PNG_FORMAT);

		if (status != B_OK) {
			fprintf(stderr, "Error converting medium icon to PNG: %s\n",
				strerror(status));
			exit(1);
		}
		
		BBitmap *largeDummy = NULL;
		largeBitmapStream.DetachBitmap(&largeDummy);
		
		fprintf(stderr, "Written large image to ");
		fprintf(stdout, "%s-icon_32.png\n", filePath.Leaf());
	}
	
	return 0;
}
