#include "FTP_Client.h"


//######################################################################################################
void    FTP_Client::begin(const char* _serverAdress,const  char* _userName, const char* _passWord, uint16_t _timeout)
{
    serverAdress = (char*)_serverAdress;
    userName = (char*)_userName;
    passWord = (char*)_passWord;    
    timeout = _timeout;
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Begin.\r\n");
    #endif
}
//######################################################################################################
bool    FTP_Client::openConnection(void)
{
    char    ans[128];
    if (client.connect(serverAdress, 21))
    {
        client.setTimeout(timeout);
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Connected.\r\n");
        #endif
    }
    else
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Can not Connect!\r\n");
        #endif
        return false;
    } 
    getFTPAnswer(ans);
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: USER %s\r\n",userName);
    #endif
    client.print(F("USER "));
    client.println(userName);
    getFTPAnswer(ans);
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: PASS %s\r\n",passWord);
    #endif
    client.print(F("PASS "));
    client.println(passWord);
    getFTPAnswer(ans);
    if(strstr(ans,"230-OK") == NULL)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Can not login! check your user,pass\r\n");
        #endif
        return false;
    }
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: SYST\r\n");
    #endif
    client.println(F("SYST"));
    getFTPAnswer(ans);
    return true;
}
//######################################################################################################
void FTP_Client::closeConnection(void)
{
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: QUIT\r\n");
    #endif
    client.println(F("QUIT"));
    client.stop();
}
//######################################################################################################
bool    FTP_Client::getFTPAnswer(char* result, int offsetStart)
{
    char thisByte;
    outCount = 0;
    unsigned long _m = millis();
    memset( outBuf, 0, sizeof(outBuf));
    if(result != NULL)
        result[0] = 0;
    while (!client.available() && millis() < _m + timeout)
        delay(1);
    delay(10);
    if(!client.available())
    {
        memset( outBuf, 0, sizeof(outBuf));
        strcpy( outBuf, "Offline");
        isConnected = false;
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline!\r\n");
        #endif
        return false;
    }
    isConnected = true;
    while (client.available())
    {
        thisByte = client.read();
        if (outCount < sizeof(outBuf))
        {
            outBuf[outCount] = thisByte;
            outCount++;
            outBuf[outCount] = 0;
        }
    }
    if(result != NULL)
    {
        for(size_t i = offsetStart; i<sizeof(outBuf); i++)
            result[i] = outBuf[i - offsetStart];
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.print("[FTP_Client] Result:\r\n");
        FTP_DebugSerial.print(outBuf);
        #endif
    }
    if(outBuf[0] == '4' || outBuf[0] == '5' )
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Not Accept!\r\n");
        #endif
        return false;
    }
    return true;
}
//######################################################################################################
bool FTP_Client::renameFile(const char* from,const char* to)
{
    char ans[128];
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: RNFR %s\r\n",from);
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    client.print(F("RNFR "));
    client.println(from);
    getFTPAnswer(ans);
    if(strstr(ans,"350") == NULL)
        return false;
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: RNTO %s\r\n",to);
    #endif
    client.print(F("RNTO "));
    client.println(to);
    getFTPAnswer(ans);
    if(strstr(ans,"250") == NULL)
        return false;
    else
        return true;
}
//######################################################################################################
bool FTP_Client::newFile (const char* fileName)
{
    char ans[128];
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: STOR %s\r\n",fileName);
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    client.print(F("STOR "));
    client.println(fileName);
    getFTPAnswer(ans);
    if(strstr(ans,"150") == NULL)
        return false;
    else
        return true;
}
//######################################################################################################
bool FTP_Client::initFile(FTP_FileType_t FTP_FileType)
{
    char ans[128];
    #ifdef FTP_DebugSerial
    if (FTP_FileType == FTP_FileType_ASCII)
        FTP_DebugSerial.printf("[FTP_Client] Send: TYPE A\r\n");
    else
        FTP_DebugSerial.printf("[FTP_Client] Send: TYPE I\r\n");
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    if (FTP_FileType == FTP_FileType_ASCII)
        client.println("TYPE A");
    else
        client.println("TYPE I");    
    getFTPAnswer(ans);
    if(strstr(ans,"200") == NULL)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Not Accept TYPE\r\n");
        #endif    
        return false;
    }
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: PSAV\r\n");
    #endif
    client.println(F("PASV"));
    getFTPAnswer(ans);
    if((strstr(ans,"227") == NULL) && (strstr(ans,"228") == NULL) && (strstr(ans,"229") == NULL))
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Not Accept PSAV\r\n");
        #endif    
        return false;
    }
    char *tStr = strtok(outBuf, "(,");
    int array_pasv[6];
    for ( int i = 0; i < 6; i++)
    {
        tStr = strtok(NULL, "(,");
        if (tStr == NULL)
        {
            #ifdef FTP_DebugSerial
            FTP_DebugSerial.println(F("[FTP_Client] Bad PASV Answer"));
            #endif
            return false;
        }
        array_pasv[i] = atoi(tStr);
    }
    unsigned int hiPort, loPort;
    hiPort = array_pasv[4] << 8;
    loPort = array_pasv[5] & 255;
    hiPort = hiPort | loPort;
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.print(F("[FTP_Client] Data port: "));
    FTP_DebugSerial.println(hiPort);
    #endif
    if (dclient.connect(serverAdress, hiPort))
    {
        dclient.setTimeout(timeout);
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.println(F("[FTP_Client] Data connection established "));
        #endif
        return true;
    }
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.println(F("[FTP_Client] Data connection Not established!"));
    #endif
    return false;
}
//######################################################################################################
bool FTP_Client::appendFile (const char* fileName)
{
    char ans[128];
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: APPE %s\r\n",fileName);
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    client.print(F("APPE "));
    client.println(fileName);
    getFTPAnswer(ans);
    if(strstr(ans,"150") == NULL)
        return false;
    else
        return true;
}
//######################################################################################################
bool FTP_Client::changeWorkDir(const char * dir)
{
    char ans[128];
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: CWD %s\r\n",dir);
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    client.print(F("CWD "));
    client.println(dir);
    getFTPAnswer(ans);
    if(strstr(ans,"250") == NULL)
        return false;
    else
        return true;
}
//######################################################################################################
bool FTP_Client::deleteFile(const char * file)
{
    char ans[128];
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: DELE %s\r\n",file);
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    client.print(F("DELE "));
    client.println(file);
    getFTPAnswer(ans);
    if(strstr(ans,"250") == NULL)
        return false;
    else
        return true;
}
//######################################################################################################
bool FTP_Client::makeDir(const char * dir)
{
    char ans[128];
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: MKD %s\r\n",dir);
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    client.print(F("MKD "));
    client.println(dir);
    getFTPAnswer(ans);
    if(strstr(ans,"257") == NULL)
        return false;
    else
        return true;    
}
//######################################################################################################
bool FTP_Client::writeData (uint8_t * data, int dataLength)
{
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Writing %d Bytes in File\r\n",dataLength);
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    if(writeClientBuffered(&dclient, data, dataLength) == false)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Writing Error!\r\n");
        #endif
        return false;
    }
    else
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Writing Done.\r\n");
        #endif
        return true;
    }
}
//######################################################################################################
bool FTP_Client::writeData (const uint8_t * data, int dataLength)
{
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Writing %d Bytes in File\r\n",dataLength);
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    if(writeClientBuffered(&dclient, data, dataLength) == false)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Writing Error!\r\n");
        #endif
        return false;
    }
    else
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Writing Done.\r\n");
        #endif
        return true;
    }
}
//######################################################################################################
bool FTP_Client::closeFile (void)
{
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Close File\r\n");
    #endif        
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    dclient.stopAll();
    return true;
}
//######################################################################################################
bool FTP_Client::write(const char * str)
{
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Writing string in File\r\n");
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    int printedByte = dclient.print(str);
    if(printedByte == (int)strlen(str))
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Writing %d Bytes in File, Done\r\n",printedByte);
        #endif
        return true;
    }
    else
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Writing only %d Bytes in File, Faild!\r\n",printedByte);
        #endif
        return false;
    }
}
//######################################################################################################
bool FTP_Client::getLastModifiedTime(const char  * fileName, char* result)
{
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: MKD %s\r\n",fileName);
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    client.print(F("MDTM "));
    client.println(fileName);
    getFTPAnswer(result, 4);
    if(result[0] == '2')
        return true;
    else
        return false;
}
//######################################################################################################
bool FTP_Client::writeClientBuffered(WiFiClient* cli, const uint8_t *data, int dataLength)
{
    size_t clientCount = 0;
    size_t cnt;
    for(int i = 0; i < dataLength;i++)
    {
        clientBuf[clientCount] = pgm_read_byte(&data[i]);
        clientCount++;
        if (clientCount > sizeof(clientBuf)-1)
        {
            cnt = cli->write(clientBuf, sizeof(clientBuf)); 
            if(cnt != sizeof(clientBuf))
                return false;
            clientCount = 0;
        }
    }
    if (clientCount > 0)
        cnt = cli->write(clientBuf, clientCount); 
    if(cnt != clientCount)
        return false;
    else
        return true;    
}
//######################################################################################################
bool FTP_Client::writeClientBuffered(WiFiClient* cli,uint8_t *data, int dataLength)
{
    size_t clientCount = 0;
    size_t cnt;
    for(int i = 0; i < dataLength;i++)
    {
        clientBuf[clientCount] = data[i];
        clientCount++;
        if (clientCount > sizeof(clientBuf)-1)
        {
            cnt = cli->write(clientBuf, sizeof(clientBuf)); 
            if(cnt != sizeof(clientBuf))
                return false;
            clientCount = 0;
        }
    }
    if (clientCount > 0)
        cnt = cli->write(clientBuf, clientCount); 
    if(cnt != clientCount)
        return false;
    else
        return true;   
}
//######################################################################################################
bool FTP_Client::contentList(const char * dir, String * list) 
{
    char _resp[ sizeof(outBuf) ];
    uint16_t _b = 0;  
    #ifdef FTP_DebugSerial
    FTP_DebugSerial.printf("[FTP_Client] Send: MLSD %s\r\n",dir);
    #endif
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    client.print(F("MLSD "));
    client.println(dir);
    getFTPAnswer(_resp);
    unsigned long _m = millis();
    while( !dclient.available() && millis() < _m + timeout) delay(1);
    while(dclient.available()) 
    {
        if( _b < 128 )
        {
            list[_b] = dclient.readStringUntil('\n');
            _b++;
        }
    }
    return true;
}
//######################################################################################################
bool FTP_Client::downloadString(const char * filename, String &str)
{
    Serial.println("Send RETR");
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    client.print(F("RETR "));
    client.println(filename);
    char _resp[ sizeof(outBuf) ];
    getFTPAnswer(_resp);
    if(strstr(_resp,"150") == NULL)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Can not open File: %s \r\n",filename);
        #endif
        return false;
    }
    unsigned long _m = millis();
    delay(100);
    while(1)
    {
        delay(1);
        if(dclient.available())
            str += dclient.readString();
        else
            break;
        if( millis() > _m + timeout )
            break;
    }    
    return true;
}
//######################################################################################################
bool FTP_Client::downloadFile(const char * filename, unsigned char * buf, size_t length)
{
    Serial.println("Send RETR");
    if(!isConnected)
    {
        #ifdef FTP_DebugSerial
        FTP_DebugSerial.printf("[FTP_Client] Offline\r\n");
        #endif
        return false;
    }
    client.print(F("RETR "));
    client.println(filename);
    char _resp[ sizeof(outBuf)];    
    getFTPAnswer(_resp);
    unsigned long _m = millis();
    while( !dclient.available() && millis() < _m + timeout) delay(1);
    while(dclient.available()) 
        dclient.readBytes(buf, length);
    return true;
}
//######################################################################################################