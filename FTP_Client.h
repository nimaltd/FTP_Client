#ifndef	FTP_CLIENT_H
#define	FTP_CLIENT_H

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define     FTP_DebugSerial         Serial

typedef enum
{
    FTP_FileType_ASCII,
    FTP_FileType_BINARY,

}FTP_FileType_t;

class FTP_Client
{
    public:
    void    begin(const char* _serverAdress,const char* _userName,const char* _passWord, uint16_t _timeout);
    bool    getFTPAnswer(char* result = NULL, int offsetStart = 0);
    bool    openConnection(void);
    void    closeConnection(void);    
    bool    initFile(FTP_FileType_t FTP_FileType);
    bool    newFile (const char* fileName);
    bool    appendFile(const char* fileName);
    bool    renameFile(const char* from,const char* to);
    bool    deleteFile(const char * file);
    bool    closeFile (void);
    bool    makeDir(const char * dir);
    bool    changeWorkDir(const char * dir);
    bool    contentList(const char * dir, String * list);
    bool    getLastModifiedTime(const char* fileName, char* result);
    bool    writeData (uint8_t * data, int dataLength);
    bool    writeData (const uint8_t * data, int dataLength);
    bool    write(const char * str);
    bool    downloadString(const char * filename, String &str);
    bool    downloadFile(const char * filename, unsigned char * buf, size_t length);

    private:
    bool        writeClientBuffered(WiFiClient* cli,const uint8_t* data, int dataLength);
    bool        writeClientBuffered(WiFiClient* cli,uint8_t* data, int dataLength);
    
    char        outBuf[128];
    uint8_t     outCount;
    WiFiClient  client;
    WiFiClient  dclient;
    char*       userName;
    char*       passWord;
    char*       serverAdress;
    bool        isConnected = false;
    uint8_t     clientBuf[1024];
    uint16_t    timeout = 10000;

};

#endif
