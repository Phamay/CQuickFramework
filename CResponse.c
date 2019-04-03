/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"


#include "php_CMyFrameExtension.h"
#include "php_CResponse.h"
#include "php_CException.h"
#include "main/SAPI.h"


//zend�෽��
zend_function_entry CResponse_functions[] = {
	PHP_ME(CResponse,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CResponse,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CResponse,setHttpCode,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResponse,send,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResponse,addHeader,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResponse,redirect,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResponse,end,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResponse,sendHttpRequest,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//ģ�鱻����ʱ
CMYFRAME_REGISTER_CLASS_RUN(CResponse)
{
	//ע��CWebApp��
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CResponse",CResponse_functions);
	CResponseCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CResponseCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_long(CResponseCe, ZEND_STRL("_httpCode"),200, ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CResponseCe, ZEND_STRL("_header"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}


//��ȡCResponse��������
void CResponse_getInstance(zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval;

	//��ȡ��̬instace����
	instanceZval = zend_read_static_property(CResponseCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//Ϊ��ʱ��ʵ��������
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zend_class_entry	**classCePP,
							*classCe;

		zval			*object,
						*saveObject;

		//��ѯ���������
		zend_hash_find(EG(class_table),"cresponse",strlen("cresponse")+1,(void**)&classCePP);
		classCe = *classCePP;

		//ʵ�����ò��
		MAKE_STD_ZVAL(object);
		object_init_ex(object,classCe);

		//ִ�й�����
		if (classCe->constructor) {
			zval constructReturn;
			zval constructVal;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, classCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//������󱣴���instance��̬����
		MAKE_STD_ZVAL(saveObject);
		ZVAL_ZVAL(saveObject,object,1,0);
		zend_update_static_property(CResponseCe,ZEND_STRL("instance"),saveObject TSRMLS_CC);
		zval_ptr_dtor(&saveObject);
		
		//����
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,0);
		zval_ptr_dtor(&object);
		return;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return;
}

PHP_METHOD(CResponse,getInstance)
{
	zval *instanceZval;

	CResponse_getInstance(&instanceZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,instanceZval,1,0);
	zval_ptr_dtor(&instanceZval);
}


PHP_METHOD(CResponse,__construct)
{
	//����֧����Ϣ
	//sendHttpHeader("Powered:CQuickFramework,Version3.0(ByExtension)" TSRMLS_CC);
}


//����HTTP CODE
int sendHttpCode(int code TSRMLS_DC){

	if (sapi_header_op(SAPI_HEADER_SET_STATUS, (void*)code TSRMLS_CC) == SUCCESS) {
		return SUCCESS;
    }
	return FAILURE;
}

//����HTTP Header
int sendHttpHeader(char *headerLine TSRMLS_DC){

	if ( sapi_add_header_ex(headerLine,strlen(headerLine)+1,1,1 TSRMLS_CC)  == SUCCESS) {
		return SUCCESS;
    }
	return FAILURE;
}

//���Ͷ���
int setRedirect(char *url,int urlLen TSRMLS_DC)
{
	sapi_header_line redirect = {0};

	//�����ض���
    redirect.line_len = spprintf(&(redirect.line), 0, "%s %s", "Location:", url);
    redirect.response_code = 0;

    if (sapi_header_op(SAPI_HEADER_REPLACE, &redirect TSRMLS_CC) == SUCCESS) {
		efree(redirect.line);
		return SUCCESS;
    }
	efree(redirect.line);
	return FAILURE;
}

//����HTTP_CODE
PHP_METHOD(CResponse,setHttpCode)
{
	long code = 200;
	int	isOver = 1;

	zval	*nowHttpCode;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l|b",&code,&isOver) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	//��ȡ��ǰ���͵�httpCode
	nowHttpCode = zend_read_property(CResponseCe,getThis(),ZEND_STRL("_httpCode"),0 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(nowHttpCode) && 200 != Z_LVAL_P(nowHttpCode) && isOver ==  0){
		RETVAL_ZVAL(getThis(),1,0);
		return;
	}

	//����״̬��
	zend_update_property_long(CResponseCe,getThis(),ZEND_STRL("_httpCode"),code TSRMLS_CC);
	RETVAL_TRUE;	
}

void CResponse_send(zval *object TSRMLS_DC)
{
	zval	*nowHttpCode,
			*nowHeader;


	//HTTP_CODE
	nowHttpCode = zend_read_property(CResponseCe,object,ZEND_STRL("_httpCode"),0 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(nowHttpCode) && Z_LVAL_P(nowHttpCode) != 200 ){
		sendHttpCode(Z_LVAL_P(nowHttpCode) TSRMLS_CC);
	}

	//�������б���
	zend_update_property_null(CResponseCe,object, ZEND_STRL("_header") TSRMLS_CC);
}

PHP_METHOD(CResponse,send)
{
	CResponse_send(getThis() TSRMLS_CC);
}


//���ͷ��Ϣ
PHP_METHOD(CResponse,addHeader)
{
	zval	*header,
			*nowHeader;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&header) == FAILURE){
		return;
	}

	if(IS_ARRAY == Z_TYPE_P(header)){

		int		i,num;
		zval	**valZval,
				*saveZval;

		num = zend_hash_num_elements(Z_ARRVAL_P(header));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(header));

		for(i = 0 ; i < num ; i++){
			zend_hash_get_current_data(Z_ARRVAL_P(header),(void**)&valZval);
			sendHttpHeader(Z_STRVAL_PP(valZval) TSRMLS_CC);
			zend_hash_move_forward(Z_ARRVAL_P(header));
		}

	}else if(IS_STRING == Z_TYPE_P(header)){

		sendHttpHeader(Z_STRVAL_P(header) TSRMLS_CC);
	}
}

PHP_METHOD(CResponse,redirect)
{
	zval	*params,
			*waitParams;
	long		code = 302,
			sendStatus = FAILURE;
	char	*url;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|l",&params,&code) == FAILURE){
		return;
	}

	//����URL
	if(IS_ARRAY == Z_TYPE_P(params)){

		CRouteParse_url(params,&url TSRMLS_CC);

	}else if(IS_STRING == Z_TYPE_P(params)){
		char *urltemp,
			 *lowerUrl;
		urltemp = estrdup(Z_STRVAL_P(params));
		lowerUrl = estrdup(urltemp);
		php_strtolower(lowerUrl,strlen(lowerUrl)+1);

		if(strstr(lowerUrl,"http://") == NULL && strstr(lowerUrl,"https://") == NULL ){
			strcat2(&url,"http://",urltemp,NULL);
		}else{
			url = estrdup(Z_STRVAL_P(params));
		}
	}

	//��SAPI����httpͷ
	sendHttpCode(code TSRMLS_CC);
	zend_update_property_long(CResponseCe,getThis(),ZEND_STRL("_httpCode"),code TSRMLS_CC);

	//���Ͷ���
	sendStatus = setRedirect(url,strlen(url)+1 TSRMLS_CC);

	efree(url);
	if(sendStatus == SUCCESS){
		RETVAL_TRUE;
		return;
	}

	RETVAL_FALSE;

}

PHP_METHOD(CResponse,end)
{
	
}

void CResponse_sendHttpRequest(char *geturl,zval *params,char *getrequsetType,zval *header,int timeout,zval **returnZval TSRMLS_DC)
{

	char *requsetUrl = "",
			*requsetType,
			*url;

	zval	*curlObject,
			*sendParams,
			*curlExecZval,
			*curlInfo,
			*curlError;

	int		headNum = 0;

	HashTable *resultTable;
	
	url = estrdup(geturl);
	requsetType = estrdup(getrequsetType);
	php_strtolower(requsetType,strlen(requsetType)+1);

	//�������
	curl_init(&curlObject);
	
	//GET��ʽ
	if(strcmp(requsetType,"get") == 0){
		char *paramsQuery;
		http_build_query(params,&paramsQuery);
		if(strstr(url,"?") == NULL){
			strcat2(&requsetUrl,url,"?",paramsQuery,NULL);
		}else{
			char tempUrl[10240];
			sprintf(tempUrl,"%s%s%s",url,"&",paramsQuery);
			requsetUrl = estrdup(tempUrl);
		}

		//����URL
		MAKE_STD_ZVAL(sendParams);
		ZVAL_STRING(sendParams,requsetUrl,1);
		curl_setopt(curlObject,10002,sendParams);
		zval_ptr_dtor(&sendParams);

		efree(requsetUrl);
		efree(paramsQuery);
	}else{
		//POST����
		char *paramsQuery = "";
		if(IS_ARRAY == Z_TYPE_P(params)){
			http_build_query(params,&paramsQuery);
		}else if(IS_STRING == Z_TYPE_P(params)){
			paramsQuery = estrdup(Z_STRVAL_P(params));
		}else{
			paramsQuery = estrdup("");
		}

		//����URL ʹ��POST
		MAKE_STD_ZVAL(sendParams);
		ZVAL_STRING(sendParams,url,1);
		curl_setopt(curlObject,10002,sendParams);
		zval_ptr_dtor(&sendParams);

		MAKE_STD_ZVAL(sendParams);
		ZVAL_STRING(sendParams,paramsQuery,1);
		curl_setopt(curlObject,10015,sendParams);
		zval_ptr_dtor(&sendParams);
		efree(paramsQuery);

		MAKE_STD_ZVAL(sendParams);
		ZVAL_BOOL(sendParams,1);
		curl_setopt(curlObject,47,sendParams);
		zval_ptr_dtor(&sendParams);
	}

	//HTTPS����֤֤�� CURLOPT_SSL_VERIFYPEER
	MAKE_STD_ZVAL(sendParams);
	ZVAL_BOOL(sendParams,0);
	curl_setopt(curlObject,64,sendParams);
	zval_ptr_dtor(&sendParams);

	//CURLOPT_SSL_VERIFYHOST
	MAKE_STD_ZVAL(sendParams);
	ZVAL_BOOL(sendParams,0);
	curl_setopt(curlObject,81,sendParams);
	zval_ptr_dtor(&sendParams);

	//CURLOPT_RETURNTRANSFER
	MAKE_STD_ZVAL(sendParams);
	ZVAL_BOOL(sendParams,1);
	curl_setopt(curlObject,19913,sendParams);
	zval_ptr_dtor(&sendParams);

	//CURLOPT_SSLVERSION
	MAKE_STD_ZVAL(sendParams);
	ZVAL_LONG(sendParams,1);
	curl_setopt(curlObject,32,sendParams);
	zval_ptr_dtor(&sendParams);


	//header
	if(IS_ARRAY == Z_TYPE_P(header)){
		headNum = zend_hash_num_elements(Z_ARRVAL_P(header));
	}
	if(headNum > 0){

		//CURLOPT_HTTPHEADER
		MAKE_STD_ZVAL(sendParams);
		ZVAL_ZVAL(sendParams,header,1,0);
		curl_setopt(curlObject,10023,sendParams);
		zval_ptr_dtor(&sendParams);

		//CURLINFO_HEADER_OUT
		MAKE_STD_ZVAL(sendParams);
		ZVAL_BOOL(sendParams,1);
		curl_setopt(curlObject,2,sendParams);
		zval_ptr_dtor(&sendParams);

	}

	//��ʱ CURLOPT_TIMEOUT
	MAKE_STD_ZVAL(sendParams);
	ZVAL_LONG(sendParams,timeout);
	curl_setopt(curlObject,13,sendParams);
	zval_ptr_dtor(&sendParams);

	//ִ��
	curl_exec(curlObject,&curlExecZval);

	//��Ϣ
	curl_getinfo(curlObject,&curlInfo);

	//����
	curl_error(curlObject,&curlError);

	//����
	MAKE_STD_ZVAL(*returnZval);
	array_init(*returnZval);
	add_assoc_zval(*returnZval,"content",curlExecZval);
	add_assoc_zval(*returnZval,"info",curlInfo);
	add_assoc_zval(*returnZval,"error",curlError);

	efree(url);
	efree(requsetType);
	zval_ptr_dtor(&curlObject);



	return;
}

//����http����
PHP_METHOD(CResponse,sendHttpRequest)
{
	zend_function *curlFe;

	char	*url,
			*requsetTypeChar = "GET";

	zval	*params = NULL,
			*requsetType = NULL,
			*header = NULL,
			*returnZval,
			*useParams,
			*useRequestType,
			*useHeader;
	
	int		urlLen;
	long		timeout = 10;

	if(zend_hash_find(EG(function_table),"curl_init",strlen("curl_init")+1,(void**)&curlFe) == FAILURE){
		zend_throw_exception(CHttpExceptionCe, "[CHttpException] System is not installed the curl extensions, unable to curl sending HTTP requests", 7001 TSRMLS_CC);
		return;
	}


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|zzzl",&url,&urlLen,&params,&requsetType,&header,&timeout) == FAILURE){
		return;
	}


	MAKE_STD_ZVAL(useParams);
	if(params == NULL || IS_NULL == Z_TYPE_P(params)){
		array_init(useParams);
	}else{
		ZVAL_ZVAL(useParams,params,1,0);
	}

	MAKE_STD_ZVAL(useRequestType);
	if(requsetType == NULL || IS_NULL == Z_TYPE_P(requsetType)){
		ZVAL_STRING(useRequestType,"GET",1);
	}else{
		if(IS_STRING == Z_TYPE_P(requsetType)){
			ZVAL_ZVAL(useRequestType,requsetType,1,0);
		}else{
			ZVAL_STRING(useRequestType,"GET",1);
		}
	}

	MAKE_STD_ZVAL(useHeader);
	if(header == NULL || IS_NULL == Z_TYPE_P(header)){
		array_init(useHeader);
	}else{
		ZVAL_ZVAL(useHeader,header,1,0);
	}


	//��������
	CResponse_sendHttpRequest(url,useParams,Z_STRVAL_P(useRequestType),useHeader,timeout,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
	zval_ptr_dtor(&useParams);
	zval_ptr_dtor(&useRequestType);
	zval_ptr_dtor(&useHeader);
}