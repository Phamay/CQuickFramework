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


#include "php_CQuickFramework.h"
#include "php_CRoute.h"
#include "php_CException.h"


//zend�෽��
zend_function_entry CRoute_functions[] = {
	PHP_ME(CRoute,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRoute,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CRoute,setController,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRoute,setAction,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRoute,setModule,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRoute,getController,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRoute,getAction,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRoute,getModule,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//ģ�鱻����ʱ
CMYFRAME_REGISTER_CLASS_RUN(CRoute)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CRoute",CRoute_functions);
	CRouteCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//�������
	zend_declare_property_null(CRouteCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CRouteCe, ZEND_STRL("controller"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CRouteCe, ZEND_STRL("action"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CRouteCe, ZEND_STRL("module"),"",ZEND_ACC_PRIVATE TSRMLS_CC);

	zend_declare_property_string(CRouteCe, ZEND_STRL("thisController"),"",ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CRouteCe, ZEND_STRL("thisAction"),"",ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CRouteCe, ZEND_STRL("thisModule"),"",ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CRouteCe, ZEND_STRL("requsetUri"),"",ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CRouteCe, ZEND_STRL("controllerObject"),ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);

	zend_declare_property_long(CRouteCe, ZEND_STRL("userRouter"),1,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS;
}

//��ȡ·�ɵ��ж���
void CRoute_getInstance(zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval;

	//��ȡ��̬instace����
	instanceZval = zend_read_static_property(CRouteCe,ZEND_STRL("instance"),1 TSRMLS_CC);

	//Ϊ��ʱ��ʵ��������
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zval			*object,
						*saveObject;

		//ʵ�����ò��
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CRouteCe);

		//ִ�й�����
		if (CRouteCe->constructor) {
			zval constructReturn;
			zval constructVal;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CRouteCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//������󱣴���instance��̬����
		zend_update_static_property(CRouteCe,ZEND_STRL("instance"),object TSRMLS_CC);

		//����
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,1);
		return;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return;
}

//�෽��:����Ӧ�ö���
PHP_METHOD(CRoute,getInstance)
{
	zval *returnZval;

	CRoute_getInstance(&returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,1);
}

//���ÿ�����
void CRoute_setController(char *controller,zval *object,zval **returnZval TSRMLS_DC)
{
	zend_update_property_string(CRouteCe,object,ZEND_STRL("controller"),controller TSRMLS_CC);
	zend_update_static_property_string(CRouteCe,ZEND_STRL("thisController"),controller TSRMLS_CC);
	return;
}

//���ÿ���
PHP_METHOD(CRoute,setController)
{
	char	*controllerName;
	int		controllerNameLen;
	zval *setStatus;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&controllerName,&controllerNameLen) == FAILURE){
		zend_throw_exception(CRouteExceptionCe, "[PluginException] Registered to [HOOKS_ROUTE_END] function in the call CRoute::getInstance()->setController(string controllerName) method, parameter error", 12001 TSRMLS_CC);
		RETVAL_FALSE;
		return;
	}
	
	CRoute_setController(controllerName,getThis(),&setStatus TSRMLS_CC);
	RETVAL_TRUE;
}

//���÷���
void CRoute_setAction(char *val,zval *object,zval **returnZval TSRMLS_DC)
{
	zend_update_property_string(CRouteCe,object,ZEND_STRL("action"),val TSRMLS_CC);
	zend_update_static_property_string(CRouteCe,ZEND_STRL("thisAction"),val TSRMLS_CC);
	return;
}

//���÷���
PHP_METHOD(CRoute,setAction)
{
	char	*val;
	int		valLen;
	zval	*setStatus;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&val,&valLen) == FAILURE){
		zend_throw_exception(CRouteExceptionCe, "[PluginException] Registered to [HOOKS_ROUTE_END] function in the call CRoute::getInstance()->setAction(string actionName) method, parameter error", 12001 TSRMLS_CC);
		RETVAL_FALSE;
		return;
	}

	CRoute_setAction(val,getThis(),&setStatus TSRMLS_CC);
	RETVAL_TRUE;
}

//����ģ��
void CRoute_setModule(char *val,zval *object,zval **returnZval TSRMLS_DC)
{
	zend_update_property_string(CRouteCe,object,ZEND_STRL("module"),val TSRMLS_CC);
	zend_update_static_property_string(CRouteCe,ZEND_STRL("thisModule"),val TSRMLS_CC);
	return;
}

//����ģ��
PHP_METHOD(CRoute,setModule)
{
	char	*val;
	int		valLen;
	zval	*setStatus;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&val,&valLen) == FAILURE){
		zend_throw_exception(CRouteExceptionCe, "[PluginException] Registered to [HOOKS_ROUTE_END] function in the call CRoute::getInstance()->setModule(string moduleName) method, parameter error", 12001 TSRMLS_CC);
		RETVAL_FALSE;
		return;
	}

	CRoute_setModule(val,getThis(),&setStatus TSRMLS_CC);
	RETVAL_TRUE;
}

//��ȡ��������
PHP_METHOD(CRoute,getController)
{
	zval *returnZval;

	returnZval = zend_read_property(CRouteCe,getThis(),ZEND_STRL("controller"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(returnZval)){
		RETVAL_STRING(Z_STRVAL_P(returnZval),1);
	}else{
		RETVAL_FALSE;
	}
}

//��ȡ������
PHP_METHOD(CRoute,getAction)
{
	zval *returnZval;

	returnZval = zend_read_property(CRouteCe,getThis(),ZEND_STRL("action"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(returnZval)){
		RETVAL_STRING(Z_STRVAL_P(returnZval),1);
	}else{
		RETVAL_FALSE;
	}
}

//��ȡģ����
PHP_METHOD(CRoute,getModule)
{
	zval *returnZval;

	returnZval = zend_read_property(CRouteCe,getThis(),ZEND_STRL("module"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(returnZval)){
		RETVAL_STRING(Z_STRVAL_P(returnZval),1);
	}else{
		RETVAL_FALSE;
	}
}

//���캯��
PHP_METHOD(CRoute,__construct)
{
	zval	*cconfigInstanceZval,
			*defaultController,
			*defaultAction,
			*defaultModule,
			*setStatus;

	//���õ���
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//��ȡĬ�Ͽ�������������ģ��
	CConfig_load("DEFAULT_CONTROLLER",cconfigInstanceZval,&defaultController TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(defaultController)){
		ZVAL_STRING(defaultController,"base",1);
	}

	CConfig_load("DEFAULT_ACTION",cconfigInstanceZval,&defaultAction TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(defaultAction)){
		ZVAL_STRING(defaultAction,"index",1);
	}

	CConfig_load("DEFAULT_MODLUE",cconfigInstanceZval,&defaultModule TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(defaultModule)){
		ZVAL_STRING(defaultModule,"www",1);
	}
	
	//����Ĭ��ֵ
	CRoute_setController(Z_STRVAL_P(defaultController),getThis(),&setStatus TSRMLS_CC);
	CRoute_setAction(Z_STRVAL_P(defaultAction),getThis(),&setStatus TSRMLS_CC);
	CRoute_setModule(Z_STRVAL_P(defaultModule),getThis(),&setStatus TSRMLS_CC);

	zval_ptr_dtor(&defaultModule);
	zval_ptr_dtor(&defaultAction);
	zval_ptr_dtor(&defaultController);
	zval_ptr_dtor(&cconfigInstanceZval);
}
