#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <AppFileInfo.h>
#include <Application.h>
#include <Bitmap.h>
#include <BitmapStream.h>
#include <File.h>
#include <Path.h>
#include <String.h>
#include <TranslatorRoster.h>

#include <fs_attr.h>

void
print_help(const char *firstarg) {
	fprintf(stderr, " Creates PNG files out of the legacy icons of an app\n");
	fprintf(stderr, "Usage: %s [-h, --help] file1 [file2 [file3 [etc...]]]\n",
		firstarg);
	exit(0);
}

bool output_piped = false;

void file_made(const char *informational, const char *name, int size) {
	fprintf(stderr, informational);
	fflush(stdout);
	fprintf(stdout, "%s-icon_%d.png\n", name, size);
	if (output_piped)
		fprintf(stderr, "%s-icon_%d.png\n", name, size);
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

	BApplication app("application/x-vnd.IconWrangler");
	output_piped = !isatty(fileno(stdout));

	while (optind < argc) {
		char *fileName = argv[optind++];
		
		BFile file(fileName, B_READ_ONLY);
		if (file.InitCheck() != B_OK) {
			fprintf(stderr, "Opening file '%s' failed: %s\n",
				fileName, strerror(file.InitCheck()));
			exit(1);
		}
		
		fprintf(stderr, "Processing '%s'\n", fileName);
		
		BPath filePath(fileName);
		
		BAppFileInfo appInfo(&file);

		BBitmap miniBitmap(BRect(0, 0, 15, 15), B_BITMAP_NO_SERVER_LINK,
			B_RGBA32);

		BBitmap largeBitmap(BRect(0, 0, 31, 31), B_BITMAP_NO_SERVER_LINK,
			B_RGBA32);
		
		if (miniBitmap.InitCheck() != B_OK) {
			fprintf(stderr, "Creating mini bitmap failed: %s\n",
				strerror(miniBitmap.InitCheck()));
			exit(1);
		}
		
		if (largeBitmap.InitCheck() != B_OK) {
			fprintf(stderr, "Creating large bitmap failed: %s\n",
				strerror(miniBitmap.InitCheck()));
			exit(1);
		}
		
		status_t status = B_OK;

		status = appInfo.GetIcon(&miniBitmap, B_MINI_ICON);
		if (status != B_OK) {
			fprintf(stderr, "Unable to get mini icon: %s\n",
				strerror(status));
			exit(1);
		}

		status = appInfo.GetIcon(&largeBitmap, B_LARGE_ICON);
		if (status != B_OK) {
			fprintf(stderr, "Unable to get large icon: %s\n",
				strerror(status));
			exit(1);
		}

		BTranslatorRoster *roster = BTranslatorRoster::Default();

		BBitmapStream miniBitmapStream(&miniBitmap);
		BString miniFileName;
		miniFileName.SetToFormat("%s-icon_16.png", filePath.Leaf());
		BFile miniFile(miniFileName, B_READ_WRITE | B_CREATE_FILE);
		
		if (miniFile.InitCheck() != B_OK) {
			fprintf(stderr, "Creating file failed: %s\n",
				strerror(miniFile.InitCheck()));
			exit(1);
		}
		
		status = roster->Translate(&miniBitmapStream, NULL, NULL, &miniFile,
			B_PNG_FORMAT);

		if (status != B_OK) {
			fprintf(stderr, "Error converting mini icon to PNG: %s\n",
				strerror(status));
			exit(1);
		}
		
		file_made("Written mini image to ", filePath.Leaf(), 16);

		BBitmap *miniDummy = NULL;
		miniBitmapStream.DetachBitmap(&miniDummy);


		BBitmapStream largeBitmapStream(&largeBitmap);
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
			fprintf(stderr, "Error converting large icon to PNG: %s\n",
				strerror(status));
			exit(1);
		}
		
		BBitmap *largeDummy = NULL;
		largeBitmapStream.DetachBitmap(&largeDummy);
		
		file_made("Written large image to ", filePath.Leaf(), 32);
	}
	
	return 0;
}
