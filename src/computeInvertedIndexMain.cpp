#include <iostream>
#include <string>
#include "InvertedIndex.h"

int main(int argc, const char** argv)
{
    std::string icdDirectory = "icd/html/";
    if (argc > 1)
    {
        icdDirectory = argv[1];
    }
    std::cout << "Creating inverted index from directory '" << icdDirectory
              << "'" << std::endl;

    InvertedIndex ii;
    ii.create_from_ICD_HTML(icdDirectory);

    std::string outfile = "ICD.ii";
    std::cout << "Writing index to file '" << outfile << "'" << std::endl;
    ii.write(outfile);
    std::cout << "Done." << std::endl;

    return 0;
}
