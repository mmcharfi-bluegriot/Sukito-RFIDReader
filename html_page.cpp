#include "html_page.h"


void sendHTMLLoginPage(WiFiClient * c){
 	c->println("HTTP/1.1 200 OK");
 	c->println("Content-type:text/html");
    //int length_msg = sizeof(html_login_page_header) + sizeof(html_login_page_sofinor1) + sizeof(html_login_page_sofinor2) + sizeof(html_login_page_sofinor3) + sizeof(html_login_page_sofinor4) + sizeof(html_login_page_form) + sizeof(html_login_page_blgrt1) + sizeof(html_login_page_blgrt2);
    //c->print("Content-Length: ");
    //c->println(length_msg);
	c->println();	


  	c->print(html_login_page_header);
  	c->print(html_login_page_form);
  	c->print(html_login_page_blgrt1);
  	c->print(html_login_page_blgrt2);
	c->println();	
}

void sendHTMLReplyPage(WiFiClient * c){
    c->println("HTTP/1.1 200 OK");
    c->println("Content-type:text/html");
    c->println();   

    c->print(html_login_page_header);
    c->print(html_login_page_reply);

    c->println();   
}
