/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * BE1 to BE2 conversion utility
 */


#include <andrewos.h>
ATK_IMPL("be1be2app.H")
#include <be1be2.H>
#include <text.H>
#include <be1be2app.H>
#include <string.h>

/*
 * Obtain list of input files
 */

static const char progName[] = "be1be2";
static char *fileList[1000];
static int fileCount;


ATKdefineRegistryNoInit(be1be2app, application);
static char *OutputName(char  *inputName);
static void Convert(char  *fileName);


boolean be1be2app::ParseArgs(int  argc, const char  **argv)
{
    int i;

    if ((this)->application::ParseArgs( argc, argv) == FALSE)
        return FALSE;

    for (fileCount = 0, i = 1; i < argc; i++)
        fileList[fileCount++] = strdup(argv[i]);

    return TRUE;
}

/*
 * Ruotines to convert one file
 */

static char *OutputName(char  *inputName)
{
    static char outName[256];
    int i;

    strcpy(outName, inputName);

    for (i = strlen(outName) - 1; i > 0; i--)
        if (outName[i] == '.')
            break;

    if (i == 0)
        strcat(outName, ".d");
    else {
        if (strcmp(outName + i + 1, "d") == 0)
            strcpy(outName + i + 1, "ez");
        else
            strcpy(outName + i + 1, "d");
    }

    return outName;
}

static void Convert(char  *fileName)
{
    char *outName;
    class text *textp;
    FILE *fp;

    textp = new text;

    fp = fopen(fileName, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: Cannot open %s (%s)\n", progName, fileName, strerror(errno));
        return;
    }

    /* If text_Read were used, the conversion would be done */
    /* automatically since text_Read does conversions.  We would */
    /* then not have any control over the user intrface. */

    if ((textp)->ReadSubString( 0, fp, FALSE) <= 0) {
        (textp)->Destroy();
        fprintf(stderr, "%s: Unable to read from %s\n", progName, fileName);
        return;
    }

    fclose(fp);

    if (! be1be2::CheckBE1(textp)) {
        (textp)->Destroy();
        fprintf(stderr, "%s: %s is not a BE1 file\n", progName, fileName);
        return;
    }

    if (be1be2::Convert(textp) == FALSE)
        fprintf(stderr, "%s: Possible conversion errors in %s\n", progName, fileName);

    outName = OutputName(fileName);

    fp = fopen(outName, "w");
    if (fp == NULL) {
        fprintf(stderr, "%s: Could not open output file %s (%s)\n", progName, outName, strerror(errno));
        (textp)->Destroy();
        return;
    }

    if ((textp)->Write( fp, 1, 1) < 0) {
        fclose(fp);
        unlink(outName);
        fprintf(stderr, "%s: Error writing output file %s (%s)\n", progName, outName, strerror(errno));
        (textp)->Destroy();
        return;
    }

    fclose(fp);
    (textp)->Destroy();

    fprintf(stderr, "%s: %s ==> %s\n", progName, fileName, outName);
    return;
}

/*
 * Convert each file
 */

boolean be1be2app::Run()
{
    int i;

    if (fileCount == 0) {
        fprintf(stderr, "%s: No files specified\n", progName);
        return TRUE;
    }

    for (i = 0; i < fileCount; i++)
        Convert(fileList[i]);

    return TRUE;
}
