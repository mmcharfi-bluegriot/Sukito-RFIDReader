#include "html_page.h"


void sendHTMLLoginPage(WiFiClient * c){
 	c->println("HTTP/1.1 200 OK");
 	c->println("Content-type:text/html");
	c->println();	
	
  	c->print(html_login_page_header);
  	c->print(html_login_page_form);
  	c->print(html_login_page_blgrt1);
  	c->print(html_login_page_blgrt2);
	c->println();	
}

void sendHTMLLoginPage2(WiFiClient * c){
 	c->println("HTTP/1.1 200 OK");
 	c->println("Content-type:text/html");
	c->println();	
	
  	c->print(html_login_page_header);
  	c->print(html_login_page_form2);
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
