#include "noteGetter.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <list>
#include <map>
#include <cstring>
#include <assert.h>
#include <string.h>

using namespace std;


/*
    Constructor for noteGetter, inFile must point to a file in the format .mid (MIDI 1.0 file format)
*/
noteGetter::noteGetter(ifstream &inFile) {
    runningStatus = false;
    lastStatusByte = "";
    stringHexMid = "";
    string str;
    inFile.seekg(0); //gotta reset the pointer to the start of the file!, When it was written, the file pointer got put to almost the end of the file
    ostringstream test;
    stringstream buffer;
    stringstream tempBitForFormatting;
    char smallPart;
    while (!inFile.eof()) {
        //read returns unformatted data in the size of one character so M, then t, then H, then D
        inFile.read(&smallPart,1); 
        tempBitForFormatting.str("");  //don't delete, this clears the tempBitForFormatting
        tempBitForFormatting << std::hex << (int) smallPart; //casts each character to an int (I don't like casting like this)
                                            //converts that int into a sting
                //then converts that string to hex
        //formatting stuff
        string stringBitForFormatting = tempBitForFormatting.str();
        if (stringBitForFormatting.length() == 1) { //first issue, parts that are like 01 get shortened to 1
            tempBitForFormatting.clear();
            tempBitForFormatting.str("0" + stringBitForFormatting);
        } else if (stringBitForFormatting.find("ffffff") !=  string::npos){
            tempBitForFormatting.clear();
            tempBitForFormatting.str(stringBitForFormatting.substr(6,2));
        }
        buffer << tempBitForFormatting.str();
        //tempFsile << tempBitForFormatting.str(); //puts the string into the temp file (mainly for debug purposes)
        //for large files, for some reason this doesn't all go in, but rest assured, the stringHexMid makes it work
        tempBitForFormatting.str(""); //don't delete, this clears the tempBitForFormatting
    }
    stringHexMid = buffer.str(); //I'm confident is this is a string representation of the hex interpretation of the binary .mid file
    inFile.seekg(0); //have to reset the pointer to the start of the file!, When it was written, the file pointer got put to almost the end of the file
}

/*
    Helper function for Variable Length Values, used in the getLengthOfVariableLengthValue and getVariableLengthValue
    increments both length and value (should be 0 at the calling of the function)
    Position is the start of the variable length value in the given stringHexMid
*/
void noteGetter::incrementLengthandValue(int position, int& length, int& value){
    bool notTheEnd = true; 
    stringstream valueString;
    while (notTheEnd) {
        string deltaPart = "";
        deltaPart = stringHexMid.substr(position + length,2);//so we load up one byte a time (2 char's worth)
        int deltaPartInt = 0;
        //convert this string representation in hex to int variable, then convert that int into binary (oh wait, ints are stored in binary)
        try {
            deltaPartInt = std::stoul(deltaPart, nullptr, 16); //converts the string note on value in hex to an int
        } catch (exception(stoul)) {
            std::cerr << "Got to the end of the file\n";
            return;
        }
        const unsigned char msbBitMask = 128; //0000 0001
        //1111 0101 would be not the end of the delta time event
    //   &1000 0000
        //result: 1000 0000  so a nonzero result means that we aren't at the end
        //0111 1111 would be the end of the delta time event
        unsigned char result = deltaPartInt & msbBitMask;
        if (result == 0) { //this means the function is at the end of the delta time event 
            notTheEnd = false;
        }
        length +=2;
        unsigned char valueOfDeltaPartInt = deltaPartInt & ~msbBitMask;
        value = value << 7; //I'm not concerned about overflow, because the max value allowed in variable length quntities is defined by the midi format
        value += valueOfDeltaPartInt; 
    }
    return;
}

/*
    Returns the length of a variable length value
    Position is the start of the variable length value in the given stringHexMid
*/
int noteGetter::getLengthOfVariableLengthValue(int position){
    int value = 0;
    int length = 0;
    incrementLengthandValue(position, length, value);
    return length;
}

/*
    Returns the value given by a varaiable length value 
    Position is the start of the variable length value in the given stringHexMid
*/
int noteGetter::getVariableLengthValue(int position) {
    int value = 0;
    int length = 0;
    incrementLengthandValue(position, length, value);
    return value;
}

/*
    Returns the position of the next message after an end of file message
    Position is the start of the current message
*/
int noteGetter::indexAfterEndOfFileMessageAt(int position) {
    runningStatus = false;
    lastStatusByte = "";
    position = -1;
    return position;
}

/*
    Returns the position of the next message after a MThd header message
    Position is the start of the current message
*/
int noteGetter::indexAfterMThdAt(int position) {
    runningStatus = false;
    lastStatusByte = "";
    //if it's an MTrk header, skip forward by the length section
    string lengthString = stringHexMid.substr(position+8,8);
    int length = stoi(lengthString);
    //  "MTrk" takes 4 bytes and that takes 8 chars to represent, so 1 byte = 2 chars
    position += length * 4.75; //converts the length in bytes to length in chars( since we are navigating a string representation of this file)
    return getNextNoteOnPosition(position);
}

/*
    Returns the position of the next message after a MTrk header message
    Position is the start of the current message
*/
int noteGetter::indexAfterMTrkAt(int position) {
    runningStatus = false;
    lastStatusByte = "";
    //MTrk header
     //go forward 8 more bytes (so 16 more characters) //this then points to a delta_time event
    //skips over the MTrk header and then the length part of the header (4 bytes after MTrk)
    // the section to go over has a variable length! in addition Mtrk headers are being lost  
    return indexAfterDeltaTimeEvent(position + 16); //we know the next thing will be a delta time event
}

/*
    Returns the position of the next message after a sysex event F0 or F7 message
    Position is the start of the current message
*/
int noteGetter::indexAfterSysexEventF0orF7(int position) {
    runningStatus = false; //don't exit running status for this type of event?
    lastStatusByte = "";
    //find the length, skip forward  
    //length is in bytes
    //1 byte = 2 char
    int lengthVal = 2 * getVariableLengthValue(position+2);
    //"getNextNotePosition F0 and F7 calling";
    lengthVal += getLengthOfVariableLengthValue(position+2) + 2; // for the F0 or F7 and  the length of the varable length value
    return indexAfterDeltaTimeEvent(position+lengthVal);
}

/*
    Returns the position of the next message after a meta event message
    Position is the start of the current message
*/
int noteGetter::indexAfterMetaEvent(int position) {
    runningStatus = false;
    lastStatusByte = "";            
    string metaEventType = stringHexMid.substr(position+2,2);
    if (metaEventType == "2f") {
            //std::cerr << "END OF TRACK\n";  
            string potentialChunkType = nextPotentialChunkType(position + 6);
            if (potentialChunkType == "4d54726b") {
            //or end of file?         
                return getNextNoteOnPosition(position + 6); //expect a track header message , we know its length 6 because these are always FF 2f 00
            } else {
                //or end of file         
                return -1;
            }
        }           
    //find the length, skip forward
    int lengthVal = 2 * getVariableLengthValue(position+4); //is this offest right
    lengthVal += getLengthOfVariableLengthValue(position+4) + 4; // for the FF nn format and the length of the varable length value
    return indexAfterDeltaTimeEvent(position+lengthVal);
}

/*
    Returns the position of the next message after a note on message
    Position is the start of the current message
*/
int noteGetter::indexAfterNextNoteOnMessage(string eventType, int position) {
    string potentialMidiEventByte = eventType.substr(0,2);
    string statusType = potentialMidiEventByte.substr(0,1);
    bool isRunningStatusMessage = not (statusType == "8" 
        or statusType == "9" 
        or statusType == "a" 
        or statusType == "b" 
        or statusType == "c"
        or statusType == "d"
        or statusType == "e"); //data bytes will never start with an 8-e
    if (isRunningStatusMessage) {       //it is the result of a running status style message (no midi event status byte) 
        if (lastStatusByte.substr(0,1) == "9") { //we are in the "note-on" status
                //std::cout << "We think this is a runningStatus message for a note on event\n"; 
            return (position - 2); //interprets this message like a 9n was there 
        }
        //else, handle like any other midi message (so skip over it by 4 or 6 characters)
        //cn and dn daya bytes are 2 characters long
        //others are 4 characters long
        if (statusType == "c" or statusType == "d") {
            return indexAfterDeltaTimeEvent(position + 2); 
        } else {
            return indexAfterDeltaTimeEvent(position + 4); 
        }
    } else {        //or it is a new midi status-byte (of type 9?)
        //should make the new running status
        string midiEventByte = potentialMidiEventByte; //potentialMidiEventByte is now known to be the midi event byte
        lastStatusByte = midiEventByte;
        runningStatus = true;
        if (midiEventByte.substr(0,1) == "9"){
                //std::cout << "-------------------------MIDI NOTE ON MESSAGE FOUND\n";
            return position; //this is the noteOnMessage FINALLY
        }
        if (statusType == "c" or statusType == "d") {
            return indexAfterDeltaTimeEvent(position + 4); //including 2 more characters (or 1 more byte for the status byte)
        } else {
            return indexAfterDeltaTimeEvent(position + 6); 
        }
    }
}

/*
    Returns the a block of the MIDI file that may be a chunk type (header message)
    Position is the start of the current message
*/
string noteGetter::nextPotentialChunkType(int position) {
    string potentialChunkType = "";
    try {
        potentialChunkType = stringHexMid.substr(position,8);
    } catch (std::exception& e) {
        throw range_error("Got to the end of the file too early at position: "+position);
    }
    return potentialChunkType;
}


/*
    Returns the position of the next message after a delta time MIDI message
    Position is the start of the current message
*/
int noteGetter::indexAfterDeltaTimeEvent (int position) {
    position += getLengthOfVariableLengthValue(position);
    return getNextNoteOnPosition(position);//we know the next event will not be a deltaTimeEvent
}

/*
    Returns the position of the next note on MIDI message in the file 
    Position is the start of the current message
    This is a type of recursive descent parser
*/
int noteGetter::getNextNoteOnPosition(int position) {//i'd like this to be a pointer, not an actual string (longer term optimisation)
    string potentialChunkType = nextPotentialChunkType(position);
    //looking at 4 byte value (represented by 8 chars)
    //std::cerr << potentialChunkType.substr(0,6) <<"\n";
    if (potentialChunkType.substr(0,8) == "ff2f0000") { //it's the end of the file! //this is a sysex event
        return indexAfterEndOfFileMessageAt(position);
    } else if (potentialChunkType == "4d546864"){ //MThd = 4d546864
        return indexAfterMThdAt(position);
    } else if (potentialChunkType == "4d54726b") { // MTrk
        return indexAfterMTrkAt(position); //we know the next thing will be a delta time event
    } else {
        //Its an event
        //it's an EVENT of type: 
        string eventType = stringHexMid.substr(position,2); 
        if (eventType == "f0" or eventType == "f7") { //sysex event F0 or F7
            return indexAfterSysexEventF0orF7(position);
        } else if (eventType == "ff") { //meta event
            return indexAfterMetaEvent(position);
        } else {    //this is a midi event of some type (with or without a midiEventByte)
            return indexAfterNextNoteOnMessage(eventType, position);
        }
    }
}

/*
    Given an empty list of numerical representation of notes, it will fill that list of notes with the notes on messages found in the .mid file
 */
int noteGetter::populateListWithNotes(list<int> &notes) {
    int count = 0;
    int position = 0;
    position = getNextNoteOnPosition(position);  
    string line, noteOnMessage, noteNumHex, velocityNumHex, channelMessage;
    while (position != -1) {
        string strLine = stringHexMid.substr(position,6);
        channelMessage = strLine.substr(1,1);
        noteOnMessage = strLine.substr(2,4);  
        noteNumHex = noteOnMessage.substr(0,2);
        velocityNumHex = noteOnMessage.substr(2,2);
        //exlude notes with velocity 0, as those are "note off" messages commonly used in lots of .mid files
        int noteNumInt = std::stoul(noteNumHex, nullptr, 16); //converts the string note on value in hex to an int
        int noteVelInt = std::stoul(velocityNumHex, nullptr, 16);
        //exlude the false notes that are lower than the bottom of a piano
        if (noteVelInt >= 1 and noteVelInt <=127 and noteNumInt >= 0 and noteNumInt <= 127) {
        //exlude midi messages that don't make sense
            notes.push_back(noteNumInt);
            count += 1;
        } 
         //we just processed a note on message, and so the next expected event is a delta time event (after 6 chars (or 3 bytes) of midi note on message)
        if (runningStatus) {
            //the +6 accounts for a status bit, if this is a running status note, its +4
            position =  indexAfterDeltaTimeEvent(position+6); //we arent expecting a deltaTime when the status is running?
        } else {                                                                
            runningStatus = true;
            position =  indexAfterDeltaTimeEvent(position+6);
        }
    } 
    return count;
}