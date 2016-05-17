/*
* Copyright (c) 2015, 2016 Christian Surlykke
*
* This file is part of the Restful Inter Process Communication (Ripc) project. 
* It is distributed under the LGPL 2.1 license.
* Please refer to the LICENSE file for a copy of the license.
*/

#include <unistd.h>
#include "errorhandling.h"
#include "json.h"
#include "jsonwriter.h"

const char* escapeStrings[32] = 
    {   
        "\u0000", "\u0001", "\u0002", "\u0003", "\u0004", "\u0005", "\u0006", "\u0007", 
        "\u0008", "\u0009", "\u000a", "\u000b", "\u000c", "\u000d", "\u000e", "\u000f", 
        "\u0010", "\u0011", "\u0012", "\u0013", "\u0014", "\u0015", "\u0016", "\u0017",
        "\u0018", "\u0019", "\u001a", "\u001b", "\u001c", "\u001d", "\u001e", "\u001f" 
    };

namespace org_restfulipc
{

    JsonWriter::JsonWriter(Json& json) :
        buffer() 
    {
        write(json);
    }

    JsonWriter::~JsonWriter()
    {
    }
    
    JsonWriter::JsonWriter() :
        buffer()
    {
    }

    void JsonWriter::writeObject(Json& json){
        buffer.write('{');
        if (json.entries->size() > 0) {
            writeString(json.entries->keyAt(0));
            buffer.write(": ");
            write(json.entries->valueAt(0));
            
            for (int i = 1; i < json.entries->size(); i++) {
                buffer.write(", ");
                writeString(json.entries->keyAt(i));
                buffer.write(": ");
                write(json.entries->valueAt(i));
            }
        }
        buffer.write('}');
    }

    void JsonWriter::write(Json& json)
    {
        if (json.mType == JsonType::Object) {
            writeObject(json);
        }
        else if (json.mType == JsonType::Array) {
            buffer.write('[');
            if (json.elements->size() > 0) {
                write(json.elements->at(0)); 
                for (int i = 1; i < json.elements->size(); i++) {
                    buffer.write(", ");
                    write(json.elements->at(i));
                }
            }
            buffer.write(']');
        }
        else if (json.mType == JsonType::String) {
            writeString(json.str);
        }
        else if (json.mType == JsonType::Number) {
                buffer.write(json.number);
        }
        else if (json.mType == JsonType::Boolean) {
                json.boolean ? buffer.write("true") : buffer.write("false");
        }
        else if (json.mType == JsonType::Null) {
                buffer.write("null");
        }
        else if (json.mType == JsonType::Undefined) {
            buffer.write("undefined");
        }
    }

    void JsonWriter::writeString(const char* string)
    {
        buffer.write('"');
        for (int i = 0; string[i]; i++) {
            if (0 <= string[i] && string[i] < 32) {
                buffer.write(escapeStrings[string[i]]);
            }
            else if (string[i] == '"') {
                buffer.write("\\\"");
            }
            else if (string[i] == '\\') {
                buffer.write("\\\\");
            }
            else {
                buffer.write(string[i]); 
            }
        }
        buffer.write('"');
    }

    LocalizingJsonWriter::LocalizingJsonWriter(Json& json, std::vector<std::string> acceptableLocales):
        JsonWriter(),
        acceptableLocales(acceptableLocales),
        lastResort(lastResort)
    {
        write(json);
    }

    LocalizingJsonWriter::~LocalizingJsonWriter()
    {
    }

    void LocalizingJsonWriter::writeObject(Json& json)
    {
        if (json.contains("_ripc:localized")) {
            bool found = false;
            for (std::string locale : acceptableLocales) {
                if (json.contains(locale)) {
                    write(json[locale]);
                    return;
                }
            }
            write(json["_ripc:localized"]);
        }
        else {
            JsonWriter::writeObject(json);
        }
    }

    FilteringJsonWriter::FilteringJsonWriter(Json& json, 
            const char* marker, 
            Json& replacements, 
            Json& fallbackReplacements, 
            const char* lastResort) :
        JsonWriter(),
        marker(marker),
        replacements(replacements),
        fallbackReplacements(fallbackReplacements),
        lastResort(lastResort)
    {
        write(json);
    }
 
    
    FilteringJsonWriter::~FilteringJsonWriter()
    {
    }

    void FilteringJsonWriter::writeString(const char* str)
    {
        if (! strncmp(marker, str, strlen(marker))) {
            if (replacements.contains(str)) {
                str = (const char*)replacements[str];
            }
            else if (fallbackReplacements.contains(str)) {
                str = (const char*) fallbackReplacements[str];
            }
            else {
                str = lastResort;
            }
        }
        buffer.write('"');
        for (const char *c = str; *c; c++) {
            buffer.write(*c); // FIXME
        }
        buffer.write('"');
    }
}

