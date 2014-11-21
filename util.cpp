
#include <stdio.h>
#include "./tiny/tinyxml.h"
#include <iostream>   // std::cout
#include <string>     // std::string, std::to_string


char inited = 0;

static TiXmlDocument doc("settings.xml");

float GetValue(char *name)
{
    double  result;
    
    if (inited == 0) {
        doc.LoadFile();
        inited = 1;
    }

    TiXmlElement *element = doc.FirstChildElement(name);
    
    if (element == NULL) {
        printf("incorrect xml name %s\n", name);
        exit(-1);
    }
    element->QueryDoubleAttribute("val", &result);
    
    return result;
}

void SetValue(char *name, float value)
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
    

    element->SetAttribute ("val", value);
    doc.SaveFile("settings.xml");
}
