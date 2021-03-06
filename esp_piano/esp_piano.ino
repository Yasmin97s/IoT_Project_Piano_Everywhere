
/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/

//This example shows how to get the documents in a document collection. This operation required Email/password, custom or OAUth2.0 authentication.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#include <json/FirebaseJson.h>
#include <SD.h>
#include <SPI.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
#include <stdlib.h>
#include <string>
//#include <sstream>
using namespace std;
/* 1. Define the WiFi credentials */
#define RXD2 16
#define TXD2 17
//#define WIFI_SSID "TechPublic"
//#define WIFI_PASSWORD ""
#define WIFI_SSID "AndroidAP"
#define WIFI_PASSWORD "edfp2567"

/* 2. Define the API Key */
#define API_KEY "AIzaSyDs5wZK9fXfq3ZaJVy_01_-mvHbvLUamMA"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "piano45-110a7"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "shukhajasmine@gmail.com"
#define USER_PASSWORD "123456"
String userEmail= "shukhajasmine@gmail.com";

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
unsigned long dataMillis = 0;

bool taskCompleted = false;
//DynamicJsonDocument doc(1024);

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

int update_isBegin(String document, String value){
        Serial.print("Commit a document (update isBegin)... ");
        String mypath = String("Lessons/" + document);
        //Serial.println("my path = " + mypath );
        //Serial.println(mypath.c_str());

        //The dyamic array of write object fb_esp_firestore_document_write_t.
        std::vector<struct fb_esp_firestore_document_write_t> writes;

        //A write object that will be written to the document.
        struct fb_esp_firestore_document_write_t update_write;
        //Set the write object write operation type.
        update_write.type = fb_esp_firestore_document_write_type_update;
        //Set the document content to write (transform)
        String content2;
        FirebaseJson js2;
        //String documentPath = "test_collection/d" + String(count);
        //js.set("fields/count/integerValue", String(count).c_str());
        //js.set("fields/random/integerValue", String(rand()).c_str());
        js2.set("fields/isBegin/booleanValue", value);
        js2.toString(content2);
        //Serial.println("update_isBegin object content is:");
        //Serial.println(content2);
        
        //Set the update document content
        update_write.update_document_content = content2.c_str();
        //Set the update document path
        update_write.update_document_path = mypath.c_str();
        //Set the document mask field paths that will be updated
        //Use comma to separate between the field paths
        //update_write.update_masks = "count,random";
        update_write.update_masks = "isBegin";
        //Set the precondition write on the document.
        //The write will fail if this is set and not met by the target document.
        //Th properties for update_write.current_document should set only one from exists or update_time
        update_write.current_document.exists = "true";
        //update_write.current_document.update_time = "2021-05-02T15:01:23Z";
        //Add a write object to a write array.
        writes.push_back(update_write);
        
        if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
        {    
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            return 0;
        }
        else
        {
            Serial.println(fbdo.errorReason());
            return -1;
        }

}

int update_studentKeys(String document, FirebaseJsonArray &arr){
        Serial.println("Commit a document (update studentKey)... ");
        String mypath = String("Lessons/" + document);
        //Serial.println("my path = " + mypath );
        //Serial.println(mypath.c_str());

        //The dyamic array of write object fb_esp_firestore_document_write_t.
        std::vector<struct fb_esp_firestore_document_write_t> writes;

        //A write object that will be written to the document.
        struct fb_esp_firestore_document_write_t update_write;
        //Set the write object write operation type.
        update_write.type = fb_esp_firestore_document_write_type_update;
        //Set the document content to write (transform)
        String content2;
        FirebaseJson js2;
        //String documentPath = "test_collection/d" + String(count);
        //js.set("fields/count/integerValue", String(count).c_str());
        //js.set("fields/random/integerValue", String(rand()).c_str());
        js2.set("fields/studentKeys/arrayValue/values", arr);
        js2.toString(content2);
        //Serial.println("update_studentKeys object content is:");
        //Serial.println(content2);
        
        //Set the update document content
        update_write.update_document_content = content2.c_str();
        //Set the update document path
        update_write.update_document_path = mypath.c_str();
        //Set the document mask field paths that will be updated
        //Use comma to separate between the field paths
        //update_write.update_masks = "count,random";
        update_write.update_masks = "studentKeys";
        //Set the precondition write on the document.
        //The write will fail if this is set and not met by the target document.
        //Th properties for update_write.current_document should set only one from exists or update_time
        update_write.current_document.exists = "true";
        //update_write.current_document.update_time = "2021-05-02T15:01:23Z";
        //Add a write object to a write array.
        writes.push_back(update_write);

        if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
        {    
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            return 0;
        }
        else
        {
            Serial.println(fbdo.errorReason());
            return -1;
        }

}


int append_item(String document, int idx, String value){
        Serial.print("Commit a document (append item to array)... ");
        String mypath = String("Lessons/" + document);
        //Serial.println("my path = " + mypath );
        //Serial.println(mypath.c_str());

        //The dyamic array of write object fb_esp_firestore_document_write_t.
        std::vector<struct fb_esp_firestore_document_write_t> writes;

        //A write object that will be written to the document.
        struct fb_esp_firestore_document_write_t transform_write;

        //Set the write object write operation type.
        transform_write.type = fb_esp_firestore_document_write_type_transform;

        //Set the document path of document to write (transform)
        transform_write.document_transform.transform_document_path = mypath.c_str();

        //Set a transformation of a field of the document.
        struct fb_esp_firestore_document_write_field_transforms_t field_transforms;

        //Set field path to write.
        field_transforms.fieldPath = "studentKeys";

        //Set the transformation type.
        field_transforms.transform_type = fb_esp_firestore_transform_type_append_missing_elements;
        
        //writes an element of array to firebase
        String content;
        FirebaseJson js;
        String path = "values/[" + String(idx) + "]/stringValue ";
        //String txt = "Hello World! " + String(count);
        js.set(path, value);

        js.toString(content);

        //Set the transformation content.
        field_transforms.transform_content = content.c_str();

        //Add a field transformation object to a write object.
        transform_write.document_transform.field_transforms.push_back(field_transforms);

        //Add a write object to a write array.
        writes.push_back(transform_write);

        if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
        {    
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            return 0;
        }
        else
        {
            Serial.println(fbdo.errorReason());
            return -1;
        }
  
}


void setup()
{

    Serial.begin(115200);
    Serial2.begin(9600,SERIAL_8N1,RXD2,TXD2);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    
    Firebase.reconnectWiFi(true);

    // clear Serial2 cache
    while(Serial2.available() > 0){
      Serial2.read();
    }         
}


void loop()
{
    if (Firebase.ready() && !taskCompleted)
    {

      //taskCompleted=true;
        String collectionId = "Lessons";
      //-------------------------------------------------------------------------
    if ( (millis() - dataMillis > 10000 || dataMillis == 0))
    {
        dataMillis = millis();

        Serial2.print("Waiting\n");

        Serial.print("Query a Firestore database... ");

        FirebaseJson query;

        //query.set(String("select/fields/[0]/fieldPath"), String("teacherUid")); //"documents/[0]/fields/teacher_notes/arrayValue/values"
        //query.set(String("select/fields/[1]/fieldPath"), String("studentEmail"));
        //query.set(String("select/fields/[2]/fieldPath"), String("studentKeys"));
        query.set(String("select/fields/[0]/fieldPath"), String("teacherKeys"));
        
        //query.set(String("select/fields/[2]/fieldPath"), String("status"));
        //String compareStudent = String("studentEmail == ") + userEmail;
        query.set(String("from/collectionId"), String("Lessons"));
        query.set("from/allDescendants", false);


        query.set("where/compositeFilter/op", "AND");
      
        query.set("where/compositeFilter/filters/[0]/fieldFilter/field/fieldPath", "studentEmail");
        query.set("where/compositeFilter/filters/[0]/fieldFilter/op", "EQUAL");
        query.set("where/compositeFilter/filters/[0]/fieldFilter/value/stringValue", userEmail);

        query.set("where/compositeFilter/filters/[1]/fieldFilter/field/fieldPath", "isBegin");
        query.set("where/compositeFilter/filters/[1]/fieldFilter/op", "EQUAL");
        query.set("where/compositeFilter/filters/[1]/fieldFilter/value/booleanValue", "false" );
       
        query.set("orderBy/field/fieldPath", "time");
        query.set("orderBy/direction", "DESCENDING");
        
        query.set("limit", 1);
        

        //The consistencyMode and consistency arguments are not assigned
        //The consistencyMode is set to fb_esp_firestore_consistency_mode_undefined by default.
        //The arguments is the consistencyMode value, see the function description at
        //https://github.com/mobizt/Firebase-ESP-Client/tree/main/src#runs-a-query

        if (Firebase.Firestore.runQuery(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, "" /* The document path */, &query /* The FirebaseJson object holds the StructuredQuery data */))
        {

          FirebaseJson documents_list;
          Serial.printf("docs:\n%s\n\n", fbdo.payload().c_str());
          
          documents_list.setJsonData(fbdo.payload().c_str());
          //String buff1;
          //documents_list.toString(buff1);
          //Serial.printf("docs:\n%s\n", buff1.c_str());
          FirebaseJsonData jsonData;
          //Get name jsonData
          documents_list.get(jsonData, "[0]/document/name"); //documents/try2/1
          String document = getValue(jsonData.stringValue, '/', 6);
          if(document == ""){
            return;
          }
          Serial.println("document="+document);
          //Get array jsonData
          documents_list.get(jsonData, "[0]/document/fields/teacherKeys/arrayValue/values"); //documents/try2/1
          //Prepare FirebaseJsonArray to take the array from FirebaseJson
          FirebaseJsonArray jsonArray;
          //Get the array data
          jsonData.getArray(jsonArray);
          String buff2;
          jsonArray.toString(buff2);
          //Serial.printf("arr:\n%s\n",buff2.c_str());
          int arraySize = jsonArray.size();
          //Serial.printf("arraySize=%d\n", arraySize);
          //Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
          //String teacher_arr[jsonArray.size()];
          int i = 0;

          //Play song on arduino:
          for ( i = 0; i < jsonArray.size(); i++)
          {
            //jsonData now used as temporary object to get the parse results
            jsonArray.get(jsonData, i);
            String note = getValue(jsonData.stringValue, '"', 3);
            Serial2.print(note);
            Serial2.print(" ");
          }
          Serial2.print("\n");

          //Serial2.print("Waiting\n");

          FirebaseJsonArray studentKeys;
          for ( i = 0; i < jsonArray.size(); i++)
          {
            //jsonData now used as temporary object to get the parse results
            jsonArray.get(jsonData, i);
            //Print its value
            String note = getValue(jsonData.stringValue, '"', 3);
            //Serial.println(note);
            bool isLegal = false;
            char mychar = 0;
            while(!isLegal){
              while(Serial2.available()<=0){
              }
              mychar = (char)Serial2.read();
              if(mychar >= 'a' && mychar <= 'm'){
                isLegal = true;
              }
            }
            
            String info = String(mychar);
            Serial.println(" Got: " + info );

            //append to studentKeys array
            //append_item(document,i, info);

            //update studentKeys array
            FirebaseJson newNote;
            newNote.add("stringValue",info);
            //String newNoteContent;
            //newNote.toString(newNoteContent);
            //Serial.println("newNote object content is:");
            //Serial.println(newNoteContent);
            studentKeys.add(newNote);
            //String studentKeysContent;
            //studentKeys.toString(studentKeysContent);
            //Serial.println("studentKeys array content is:");
            //Serial.println(studentKeysContent);
            update_studentKeys(document, studentKeys);

            if(i == 0){
              update_isBegin(document, "true");
            }

            if(info!=note){
             if(i == 0){
               Serial2.print("all wrong\n");
             } else {
               Serial2.print("not matched\n");
             }
             Serial.println(note + " != " + info );
             break;
            }
             
          }
          //todo: update points
          
          
          if(i == jsonArray.size() && i != 0)
          {
            //taskCompleted = true;
            Serial.println("Well done!");
            Serial2.print("matched\n");
          }
          
        }
        else{
            Serial.print("Query failed: ");
            Serial.println(fbdo.errorReason());
        }
  }           
  }
}
