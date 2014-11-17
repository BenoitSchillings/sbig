#include <stdio.h>
#include "./tiny/tinyxml.h"


char inited = 0;

static TiXmlDocument doc("settings.xml");

float GetValue(char *name)
{
    if (inited == 0) {
        doc.LoadFile();
        inited = 1;
    }

    TiXmlElement *element = doc.FirstChildElement(name);
    
    if (element == NULL) {
        printf("incorrect xml name %s\n", name);
        exit(-1);
    }
    
    float result = atof(element->GetText());
    
    return result;
}
