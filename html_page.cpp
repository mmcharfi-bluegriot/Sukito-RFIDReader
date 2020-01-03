#include "html_page.h"


void sendHTMLLoginPage_first_configuration(WiFiClient * c){
 	c->println("HTTP/1.1 200 OK");
 	c->println("Content-type:text/html");
	c->println();	
	
  	c->print(html_login_page_header);
  	c->print(html_login_page_form);
  	c->print(html_login_page_blgrt1);
  	c->print(html_login_page_blgrt2);
	c->println();	
}

void sendHTMLLoginPage_new_configuration(WiFiClient * c, String ssid, String id){
 	c->println("HTTP/1.1 200 OK");
 	c->println("Content-type:text/html");
	c->println();	
	
  	c->print(html_login_page_header);
  	c->print(html_new_login_page_form_1);
	c->print(id);
	c->print(html_new_login_page_form_2);
	c->print(ssid);
	c->print(html_new_login_page_form_3);
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
