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
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"

#include "php_CMyFrameExtension.h"
#include "php_CActiveRecord.h"
#include "php_CException.h"
#include "php_CDatabase.h"
#include "php_CModel.h"
#include "php_CValidate.h"




//zend�෽��
zend_function_entry CActiveRecord_functions[] = {
	PHP_ME(CActiveRecord,findAll,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,findByAttributes,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,find,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,asArray,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,getKey,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,current,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,save,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,add,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,count,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,update,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,delete,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,getLastError,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CActiveRecord,tableName,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	{NULL, NULL, NULL}
};

//ģ�鱻����ʱ
CMYFRAME_REGISTER_CLASS_RUN(CActiveRecord)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CActiveRecord",CActiveRecord_functions);
	CActiveRecordCe = zend_register_internal_class_ex(&funCe,CModelCe,NULL TSRMLS_CC);

	zend_declare_property_null(CActiveRecordCe, ZEND_STRL("arResult"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CActiveRecordCe, ZEND_STRL("condition"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CActiveRecordCe, ZEND_STRL("params"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CActiveRecordCe, ZEND_STRL("where"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CActiveRecordCe, ZEND_STRL("_lastError"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

int CActiveRecord_insert(char *config,char *table,zval *value,zval **exception TSRMLS_DC)
{
	zval	*CDatabase,
			callQueryReturn,
			callinsertReturn,
			callValueReturn,
			callExecuteReturn;

	int		callStatus = 0;

	//getInstance
	CDatabase_getInstance(0,config,&CDatabase TSRMLS_CC);
	
	//call from
	MODULE_BEGIN
		zval	callQuery,
				*callQueryParamsList[1];
		MAKE_STD_ZVAL(callQueryParamsList[0]);
		ZVAL_STRING(callQueryParamsList[0],table,1);
		INIT_ZVAL(callQuery);
		ZVAL_STRING(&callQuery,"from",0);
		call_user_function(NULL, &CDatabase, &callQuery, &callQueryReturn, 1, callQueryParamsList TSRMLS_CC);
		zval_ptr_dtor(&callQueryParamsList[0]);
	MODULE_END

	//call insert
	MODULE_BEGIN
		zval	callQuery;
		INIT_ZVAL(callQuery);
		ZVAL_STRING(&callQuery,"insert",0);
		call_user_function(NULL, &CDatabase, &callQuery, &callinsertReturn, 0, NULL TSRMLS_CC);
	MODULE_END

	//call value
	MODULE_BEGIN
		zval	callQuery,
				*callQueryParamsList[1];
		MAKE_STD_ZVAL(callQueryParamsList[0]);
		ZVAL_ZVAL(callQueryParamsList[0],value,1,0);
		INIT_ZVAL(callQuery);
		ZVAL_STRING(&callQuery,"value",0);
		call_user_function(NULL, &CDatabase, &callQuery, &callValueReturn, 1, callQueryParamsList TSRMLS_CC);
		zval_ptr_dtor(&callQueryParamsList[0]);
	MODULE_END

	//call execute
	MODULE_BEGIN
		zval	callQuery;
		INIT_ZVAL(callQuery);
		ZVAL_STRING(&callQuery,"execute",0);
		call_user_function(NULL, &CDatabase, &callQuery, &callExecuteReturn, 0, NULL TSRMLS_CC);
	MODULE_END

	MAKE_STD_ZVAL(*exception);
	if(EG(exception)){
		zend_class_entry	*exceptionCe;
		zval				*exceptionMessage,
							**message;
		exceptionCe = Z_OBJCE_P(EG(exception));
		exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);
		ZVAL_STRING(*exception,Z_STRVAL_P(exceptionMessage),1);
		callStatus = 0;
		//php 5.4 after clear cache will check this val is equeue handle,and throw Attempt to destruct pending exception
		Z_OBJ_HANDLE_P(EG(exception)) = 0;		
		zend_clear_exception(TSRMLS_C);
	}else{
		ZVAL_STRING(*exception,"",1);
		callStatus = 1;
	}

	zval_ptr_dtor(&CDatabase);
	zval_dtor(&callQueryReturn);
	zval_dtor(&callinsertReturn);
	zval_dtor(&callValueReturn);
	zval_dtor(&callExecuteReturn);
	return callStatus;
}

void CActiveRecord_getDBSQL(char *config,char *sql,zval **returnZval TSRMLS_DC)
{
	zval	*CDatabase,
			*processRetrun,
			*asArrayData,
			processAsArray,
			callQueryReturn;

	CDatabase_getInstance(0,config,&CDatabase TSRMLS_CC);

	//call query
	MODULE_BEGIN
		zval	callQuery,
				*callQueryParamsList[1];
		MAKE_STD_ZVAL(callQueryParamsList[0]);
		ZVAL_STRING(callQueryParamsList[0],sql,1);
		INIT_ZVAL(callQuery);
		ZVAL_STRING(&callQuery,"query",0);
		call_user_function(NULL, &CDatabase, &callQuery, &callQueryReturn, 1, callQueryParamsList TSRMLS_CC);
		processRetrun = &callQueryReturn;
		zval_ptr_dtor(&callQueryParamsList[0]);
	MODULE_END

	//call asArray
	MODULE_BEGIN
		zval	callQuery;
		INIT_ZVAL(callQuery);
		ZVAL_STRING(&callQuery,"asArray",0);
		call_user_function(NULL, &processRetrun, &callQuery, &processAsArray, 0, NULL TSRMLS_CC);
		asArrayData = &processAsArray;
	MODULE_END

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,asArrayData,1,0);

	zval_ptr_dtor(&CDatabase);
	zval_dtor(&callQueryReturn);
	zval_dtor(&processAsArray);
}

void CActiveRecord_getSQL(char *sql,zval **returnZval TSRMLS_DC)
{
	zval	*CDatabase,
			*processRetrun,
			*asArrayData,
			processAsArray,
			callQueryReturn;

	CDatabase_getInstance(0,"main",&CDatabase TSRMLS_CC);

	//call query
	MODULE_BEGIN
		zval	callQuery,
				*callQueryParamsList[1];
		MAKE_STD_ZVAL(callQueryParamsList[0]);
		ZVAL_STRING(callQueryParamsList[0],sql,1);
		INIT_ZVAL(callQuery);
		ZVAL_STRING(&callQuery,"query",0);
		call_user_function(NULL, &CDatabase, &callQuery, &callQueryReturn, 1, callQueryParamsList TSRMLS_CC);
		processRetrun = &callQueryReturn;
		zval_ptr_dtor(&callQueryParamsList[0]);
	MODULE_END

	//call asArray
	MODULE_BEGIN
		zval	callQuery;
		INIT_ZVAL(callQuery);
		ZVAL_STRING(&callQuery,"asArray",0);
		call_user_function(NULL, &processRetrun, &callQuery, &processAsArray, 0, NULL TSRMLS_CC);
		asArrayData = &processAsArray;
	MODULE_END

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,asArrayData,1,0);

	zval_ptr_dtor(&CDatabase);
	zval_dtor(&callQueryReturn);
	zval_dtor(&processAsArray);
}

//��ȡ���й�������
void CActiveRecord_getAllSetAttribute(zval *object,zval **returnZval TSRMLS_DC)
{
	zval *addDefaultZval,
		 *selfArray,
		 **thisVal,
		 *defaultAttrZval;
	int num,
		i;

	char *key;
	ulong ukey;

	MAKE_STD_ZVAL(*returnZval);
	array_init(*returnZval);

	MAKE_STD_ZVAL(defaultAttrZval);
	array_init(defaultAttrZval);


	//activeRecord�еı���
/*
#if ( PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION <= 3 )
	num = zend_hash_num_elements((HashTable*)&((Z_OBJCE_P(object)->default_properties)) );
	zend_hash_internal_pointer_reset((HashTable*)&((Z_OBJCE_P(object)->default_properties)) );
#else
	num = zend_hash_num_elements((HashTable*)&((Z_OBJCE_P(object)->default_properties_table)));
	zend_hash_internal_pointer_reset((HashTable*)&((Z_OBJCE_P(object)->default_properties_table)));
#endif
	for(i = 0 ; i< num; i++){

#if ( PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION <= 3 )
		zend_hash_get_current_key((HashTable*)&((Z_OBJCE_P(object)->default_properties)),&key,&ukey,0);
#else
		zend_hash_get_current_key((HashTable*)&((Z_OBJCE_P(object)->default_properties_table)),&key,&ukey,0);
#endif

		if(strlen(key) > 0){
			add_next_index_string(defaultAttrZval,key,1);
		}
#if ( PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION <= 3 )
		zend_hash_move_forward((HashTable*)&((Z_OBJCE_P(object)->default_properties)));
#else
		zend_hash_move_forward((HashTable*)&((Z_OBJCE_P(object)->default_properties_table)));
#endif
	}
*/

	//�����е����б���
	MAKE_STD_ZVAL(selfArray);
	ZVAL_ZVAL(selfArray,object,1,0);
	convert_to_array(selfArray);

	num = zend_hash_num_elements(Z_ARRVAL_P(selfArray));
	for(i = 0 ; i< num; i++){

		zend_hash_get_current_key(Z_ARRVAL_P(selfArray),&key,&ukey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(selfArray),(void**)&thisVal);

		if(in_array(key,defaultAttrZval)){
			zend_hash_move_forward(Z_ARRVAL_P(selfArray));
			continue;
		}
		
		if(strstr(key,"CActiveRecord") != NULL || strstr(key,"*") != NULL){
			zend_hash_move_forward(Z_ARRVAL_P(selfArray));
			continue;
		}

		if(strlen(key) == 0){
			zend_hash_move_forward(Z_ARRVAL_P(selfArray));
			continue;
		}

		if(strcmp(key,"pageRows") == 0){
			zend_hash_move_forward(Z_ARRVAL_P(selfArray));
			continue;
		}


		if(IS_STRING == Z_TYPE_PP(thisVal)){

			add_assoc_string(*returnZval,key,Z_STRVAL_PP(thisVal),1);

		}else if(IS_LONG == Z_TYPE_PP(thisVal)){

			add_assoc_long(*returnZval,key,Z_LVAL_PP(thisVal));

		}else if(IS_DOUBLE == Z_TYPE_PP(thisVal)){

			add_assoc_double(*returnZval,key,Z_DVAL_PP(thisVal));

		}else{
			zend_hash_move_forward(Z_ARRVAL_P(selfArray));
			continue;
		}


		//�������Ӽ�¼
		zend_hash_move_forward(Z_ARRVAL_P(selfArray));
	}


	zval_ptr_dtor(&selfArray);
	zval_ptr_dtor(&defaultAttrZval);
}

//�෽��:����Ӧ�ö���
PHP_METHOD(CActiveRecord,findAll)
{
	zval *tableNameZval,
		 *configZval,
		 *cconfigInstanceZval,
		 *CDatabase,
		 callAction,
		 callReturn,
		 *paramsList[3],
		 *functionReturn,
		 *saveArZval,
		 *saveCopy;

	char *tableName = "",
		 *tablePreFix = "",
		 *tableAllName = "",
		 *sql = "";


	//����������
	char *where = "";
	int  whereLen = 0;
	zval *condition = NULL;
	int isAsArray;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|szb",&where,&whereLen,&condition,&isAsArray) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord->find] Parameter error", 9001 TSRMLS_CC);
		return;
	}

	zend_update_property_string(CActiveRecordCe,getThis(),ZEND_STRL("condition"),where TSRMLS_CC);
	if(condition != NULL && IS_ARRAY == Z_TYPE_P(condition)){
		zend_update_property(CActiveRecordCe,getThis(),ZEND_STRL("params"),condition TSRMLS_CC);
	}


	//���ö���
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//��ȡ����
	MODULE_BEGIN
		zval	callAction,
				callReturn;

		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"tableName",0);
		call_user_function(NULL, &getThis(), &callAction, &callReturn,0, NULL TSRMLS_CC);
		tableNameZval = &callReturn;
		if(IS_STRING != Z_TYPE_P(tableNameZval) || strlen(Z_STRVAL_P(tableNameZval)) <= 0){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord] method to realize the tableName () abstract methods and return to the operation of the database table name", 9001 TSRMLS_CC);
			return;
		}
		tableName = estrdup(Z_STRVAL_P(tableNameZval));
		zval_dtor(&callReturn);
	MODULE_END


	//��ȡ�����ļ�
	MODULE_BEGIN
		zval	*dbSlaveConfigs,
				**tablePreZval;

		CConfig_load("DB.main.slaves",cconfigInstanceZval,&dbSlaveConfigs TSRMLS_CC);
		if(IS_ARRAY != Z_TYPE_P(dbSlaveConfigs)){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord] Failed to get the configuration[DB->main->master]", 9001 TSRMLS_CC);
			return;
		}

		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(dbSlaveConfigs),"tablePrefix",strlen("tablePrefix")+1,(void**)&tablePreZval) && IS_STRING == Z_TYPE_PP(tablePreZval)){
			tablePreFix = estrdup(Z_STRVAL_PP(tablePreZval));
		}else{
			tablePreFix = estrdup("");
		}
		zval_ptr_dtor(&dbSlaveConfigs);
	MODULE_END


	//��������
	strcat2(&tableAllName,tablePreFix,tableName,NULL);
	efree(tableName);
	efree(tablePreFix);


	//CDatabase::getInstance����
	MODULE_BEGIN
		CDatabase_getInstance(0,"main",&CDatabase TSRMLS_CC);
	MODULE_END

	MAKE_STD_ZVAL(saveArZval);
	ZVAL_NULL(saveArZval);

	//��װSQL
	if(whereLen <= 0 ){

		smart_str sql = {0};

		smart_str_appends(&sql,"SELECT * FROM ");
		smart_str_appends(&sql,tableAllName);
		smart_str_appends(&sql," LIMIT 100000");
		smart_str_0(&sql);

		//����query����
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"query",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],sql.c,1);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
		functionReturn = &callReturn;
		zval_ptr_dtor(&paramsList[0]);
		smart_str_free(&sql);

		ZVAL_ZVAL(saveArZval,functionReturn,1,0);
		zval_dtor(&callReturn);

	}else{

		zval *prepareReturn,
			 dataReturn;

		smart_str sql = {0};
		smart_str_appends(&sql,"SELECT * FROM ");
		smart_str_appends(&sql,tableAllName);
		smart_str_appends(&sql," WHERE ");
		smart_str_appends(&sql,where);
		smart_str_appends(&sql," LIMIT 100000");
		smart_str_0(&sql);

		//����prepare����
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"prepare",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],sql.c,1);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
		prepareReturn = &callReturn;
		zval_ptr_dtor(&paramsList[0]);
		smart_str_free(&sql);


		//�Ե��ý���ٴ�ִ��execute����
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"execute",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],condition,1,0);
		call_user_function(NULL, &prepareReturn, &callAction, &dataReturn, 1, paramsList TSRMLS_CC);
		functionReturn = &dataReturn;
		zval_ptr_dtor(&paramsList[0]);

		ZVAL_ZVAL(saveArZval,functionReturn,1,0);

		zval_dtor(&callReturn);
		zval_dtor(&dataReturn);
	}


	//��ȡ���
	if(IS_OBJECT == Z_TYPE_P(saveArZval)){
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"asArray",0);
		call_user_function(NULL, &functionReturn, &callAction, &callReturn, 0, NULL TSRMLS_CC);
		saveCopy = &callReturn;
		zval_ptr_dtor(&saveArZval);
		MAKE_STD_ZVAL(saveArZval);
		ZVAL_ZVAL(saveArZval,saveCopy,1,0);
		zval_dtor(&callReturn);
	}


	//��saveArZval�������������
	zend_update_property(CActiveRecordCe,getThis(),ZEND_STRL("arResult"),saveArZval TSRMLS_CC);

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&CDatabase);
	efree(tableAllName);
	zval_ptr_dtor(&saveArZval);
	RETVAL_ZVAL(getThis(),1,0);
}


PHP_METHOD(CActiveRecord,findByAttributes)
{
	zval *tableNameZval,
		 *configZval,
		 *cconfigInstanceZval,
		 *CDatabase,
		 callAction,
		 callReturn,
		 *paramsList[3],
		 param1,
		 param2,
		 param3,
		 *functionReturn,
		 *saveArZval,
		 *funcCopy,
		 *saveCopy;

	char *tableName = "",
		 *tablePreFix = "",
		 *tableAllName = "",
		 *sql = "";

	zval *where;

	int isAsArray = 0;

	paramsList[0] = &param1;
	paramsList[1] = &param2;
	paramsList[2] = &param3;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|b",&where,&isAsArray) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord->findByAttributes] Parameter error", 9001 TSRMLS_CC);
		return;
	}

	zend_update_property(CActiveRecordCe,getThis(),ZEND_STRL("where"),where TSRMLS_CC);

	//���ö���
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//��ȡ����
	MODULE_BEGIN
		zval	callAction,
				callReturn;

		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"tableName",0);
		call_user_function(NULL, &getThis(), &callAction, &callReturn,0, NULL TSRMLS_CC);
		tableNameZval = &callReturn;
		if(IS_STRING != Z_TYPE_P(tableNameZval) || strlen(Z_STRVAL_P(tableNameZval)) <= 0){
			zend_throw_exception(CDbExceptionCe, "[QueryException]Call [CActiveRecord] method to realize the tableName () abstract methods and return to the operation of the database table name", 9001 TSRMLS_CC);
			return;
		}
		tableName = estrdup(Z_STRVAL_P(tableNameZval));
		zval_dtor(&callReturn);
	MODULE_END

	//��ȡ�����ļ�
	MODULE_BEGIN
		zval	*dbSlaveConfigs,
				**tablePreZval;

		CConfig_load("DB.main.master",cconfigInstanceZval,&dbSlaveConfigs TSRMLS_CC);
		if(IS_ARRAY != Z_TYPE_P(dbSlaveConfigs)){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord] Failed to get the configuration[DB->main->master]", 9001 TSRMLS_CC);
			return;
		}
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(dbSlaveConfigs),"tablePrefix",strlen("tablePrefix")+1,(void**)&tablePreZval) && IS_STRING == Z_TYPE_PP(tablePreZval)){
			tablePreFix = estrdup(Z_STRVAL_PP(tablePreZval));
		}else{
			tablePreFix = estrdup("");
		}
		zval_ptr_dtor(&dbSlaveConfigs);
	MODULE_END

	//��������
	strcat2(&tableAllName,tablePreFix,tableName,NULL);


	//CDatabase::getInstance����
	MODULE_BEGIN
		CDatabase_getInstance(0,"main",&CDatabase TSRMLS_CC);
	MODULE_END

	//ִ��select
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"select",0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);
	zval_dtor(&callReturn);

	//ִ��from
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"from",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_STRING(paramsList[0],tableName,1);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
	zval_dtor(&callReturn);
	zval_ptr_dtor(&paramsList[0]);

	//ִ��where
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"where",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_ZVAL(paramsList[0],where,1,0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
	zval_dtor(&callReturn);
	zval_ptr_dtor(&paramsList[0]);

	//ִ��execute
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"execute",0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);
	functionReturn = &callReturn;

	//��ȡ���
	if(IS_OBJECT == Z_TYPE_P(functionReturn)){
		zval thisFunReturn;
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"asArray",0);
		call_user_function(NULL, &functionReturn, &callAction, &thisFunReturn, 0, NULL TSRMLS_CC);
		saveCopy = &thisFunReturn;
		MAKE_STD_ZVAL(saveArZval);
		ZVAL_ZVAL(saveArZval,saveCopy,1,0);
		zval_dtor(&thisFunReturn);
	}else{
		MAKE_STD_ZVAL(saveArZval);
		array_init(saveArZval);
	}

	//��saveArZval�������������
	zend_update_property(CActiveRecordCe,getThis(),ZEND_STRL("arResult"),saveArZval TSRMLS_CC);

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&saveArZval);
	zval_ptr_dtor(&CDatabase);
	efree(tablePreFix);
	efree(tableName);
	efree(tableAllName);
	zval_dtor(&callReturn);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CActiveRecord,find)
{
	zval *tableNameZval,
		 *configZval,
		 *cconfigInstanceZval,
		 *CDatabase,
		 callAction,
		 callReturn,
		 *paramsList[3],
		 param1,
		 param2,
		 param3,
		 *functionReturn,
		 *saveArZval,
		 *saveCopy;

	char *tableName = "",
		 *tablePreFix = "",
		 *tableAllName = "",
		 *sql = "";


	//����������
	char *where = "";
	int  whereLen = 0;
	zval *condition;
	int isAsArray;

	paramsList[0] = &param1;
	paramsList[1] = &param2;
	paramsList[2] = &param3;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz|b",&where,&whereLen,&condition,&isAsArray) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord->find] Parameter error", 9001 TSRMLS_CC);
		return;
	}


	zend_update_property_string(CActiveRecordCe,getThis(),ZEND_STRL("condition"),where TSRMLS_CC);
	zend_update_property(CActiveRecordCe,getThis(),ZEND_STRL("params"),condition TSRMLS_CC);
	
	//���ö���
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//��ȡ����
	MODULE_BEGIN
		zval	callAction,
				callReturn;

		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"tableName",0);
		call_user_function(NULL, &getThis(), &callAction, &callReturn,0, NULL TSRMLS_CC);
		tableNameZval = &callReturn;
		if(IS_STRING != Z_TYPE_P(tableNameZval) || strlen(Z_STRVAL_P(tableNameZval)) <= 0){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord] method to realize the tableName () abstract methods and return to the operation of the database table name", 9001 TSRMLS_CC);
			return;
		}
		tableName = estrdup(Z_STRVAL_P(tableNameZval));
		zval_dtor(&callReturn);
	MODULE_END


	//��ȡ�����ļ�
	MODULE_BEGIN
		zval	*dbSlaveConfigs,
				**tablePreZval;

		CConfig_load("DB.main.slaves",cconfigInstanceZval,&dbSlaveConfigs TSRMLS_CC);
		if(IS_ARRAY != Z_TYPE_P(dbSlaveConfigs)){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord] Failed to get the configuration[DB->main->master]", 9001 TSRMLS_CC);
			return;
		}

		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(dbSlaveConfigs),"tablePrefix",strlen("tablePrefix")+1,(void**)&tablePreZval) && IS_STRING == Z_TYPE_PP(tablePreZval)){
			tablePreFix = estrdup(Z_STRVAL_PP(tablePreZval));
		}else{
			tablePreFix = estrdup("");
		}
		zval_ptr_dtor(&dbSlaveConfigs);
	MODULE_END


	//��������
	strcat2(&tableAllName,tablePreFix,tableName,NULL);
	efree(tableName);
	efree(tablePreFix);


	//CDatabase::getInstance����
	MODULE_BEGIN
		CDatabase_getInstance(0,"main",&CDatabase TSRMLS_CC);
	MODULE_END

	MAKE_STD_ZVAL(saveArZval);
	ZVAL_NULL(saveArZval);

	//��װSQL
	MODULE_BEGIN
		zval *prepareReturn,
			 dataReturn;

		smart_str sql = {0};
		smart_str_appends(&sql,"SELECT * FROM ");
		smart_str_appends(&sql,tableAllName);
		smart_str_appends(&sql," WHERE ");
		smart_str_appends(&sql,where);
		smart_str_appends(&sql," LIMIT 100000");
		smart_str_0(&sql);

		//����prepare����
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"prepare",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],sql.c,1);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
		prepareReturn = &callReturn;
		zval_ptr_dtor(&paramsList[0]);
		smart_str_free(&sql);


		//�Ե��ý���ٴ�ִ��execute����
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"execute",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],condition,1,0);
		call_user_function(NULL, &prepareReturn, &callAction, &dataReturn, 1, paramsList TSRMLS_CC);
		functionReturn = &dataReturn;
		zval_ptr_dtor(&paramsList[0]);

		ZVAL_ZVAL(saveArZval,functionReturn,1,0);

		zval_dtor(&callReturn);
		zval_dtor(&dataReturn);
	MODULE_END

	//��ȡ���
	if(IS_OBJECT == Z_TYPE_P(saveArZval)){
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"asArray",0);
		call_user_function(NULL, &functionReturn, &callAction, &callReturn, 0, NULL TSRMLS_CC);
		saveCopy = &callReturn;
		zval_ptr_dtor(&saveArZval);
		MAKE_STD_ZVAL(saveArZval);
		ZVAL_ZVAL(saveArZval,saveCopy,1,0);
		zval_dtor(&callReturn);
	}

	//��saveArZval�������������
	zend_update_property(CActiveRecordCe,getThis(),ZEND_STRL("arResult"),saveArZval TSRMLS_CC);

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&CDatabase);
	efree(tableAllName);
	zval_ptr_dtor(&saveArZval);
	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CActiveRecord,asArray)
{
	zval *val,
		 *returnZval,
		 **thisVal;

	val = zend_read_property(CActiveRecordCe,getThis(),ZEND_STRL("arResult"), 0 TSRMLS_CC);
	ZVAL_ZVAL(return_value,val,1,0);
}

PHP_METHOD(CActiveRecord,getKey)
{
	char *key,
		 *thisKeyValChar;
	int keyLen;
	zval *val,
		 **thisVal,
		 *thisSaveVal,
		 *returnZval,
		 **thisKeyVal;
	int i,num;
	HashTable *thisReturn;

	array_init(return_value);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[ResultException] Call [CResult->getKey(string key)] Parameter error", 6010 TSRMLS_CC);
		return;
	}

	val = zend_read_property(CActiveRecordCe,getThis(),ZEND_STRL("arResult"), 0 TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(val)){
		return;
	}

	num = zend_hash_num_elements(Z_ARRVAL_P(val));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(val));

	for(i = 0 ; i < num ; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(val),(void**)&thisVal);

		//key��ֵ
		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),key,strlen(key)+1,(void**)&thisKeyVal)){

			zval *saveZval;
			MAKE_STD_ZVAL(saveZval);
			ZVAL_ZVAL(saveZval,*thisVal,1,0);

			if(Z_TYPE_PP(thisKeyVal) == IS_STRING){

				add_assoc_zval(return_value,Z_STRVAL_PP(thisKeyVal),saveZval);

			}else if(Z_TYPE_PP(thisKeyVal) == IS_LONG){

				add_index_zval(return_value,Z_LVAL_PP(thisKeyVal),saveZval);
			}

		}else{

			zend_throw_exception(CDbExceptionCe, "[ResultException] Call [CResult->getKey(string key)] The result set does not exist the field", 6011 TSRMLS_CC);
			return;
		}
	
		zend_hash_move_forward(Z_ARRVAL_P(val));
	}
}

PHP_METHOD(CActiveRecord,current)
{
	zval *val,
		 **thisVal;
	int i,num;

	val = zend_read_property(CActiveRecordCe,getThis(),ZEND_STRL("arResult"), 0 TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(val)){
		array_init(return_value);
		return;
	}

	num = zend_hash_num_elements(Z_ARRVAL_P(val));
	if(num == 0){
		array_init(return_value);
		return;
	}

	//��ȡ��һ��ֵ
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(val));
	zend_hash_get_current_data(Z_ARRVAL_P(val),(void**)&thisVal);
	convert_to_array_ex(thisVal);
	RETVAL_ZVAL(*thisVal,1,0);
}

PHP_METHOD(CActiveRecord,save)
{
	zval *publicAttributeList;

	zval *tableNameZval,
		 *configZval,
		 *cconfigInstanceZval,
		 *CDatabase,
		 callAction,
		 callReturn,
		 *paramsList[3],
		 param1,
		 param2,
		 param3,
		 *functionReturn,
		 *saveArZval,
		 *saveCopy,
		 *conditionZval,
		 *paramsZval,
		 *whereZval;

	char *tableName = "",
		 *tablePreFix = "",
		 *tableAllName = "",
		 *sql = "";


	//����������
	char *where = "";
	int  whereLen = 0;
	zval *condition;
	int isAsArray;

	paramsList[0] = &param1;
	paramsList[1] = &param2;
	paramsList[2] = &param3;

	ZVAL_ZVAL(return_value,getThis(),1,0);

	//���ö���
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//��ȡ����
	MODULE_BEGIN
		zval	callActionTemp,
				callReturnTemp;

		INIT_ZVAL(callActionTemp);
		ZVAL_STRING(&callActionTemp,"tableName",0);
		call_user_function(NULL, &getThis(), &callActionTemp, &callReturnTemp,0, NULL TSRMLS_CC);
		tableNameZval = &callReturnTemp;
		if(IS_STRING != Z_TYPE_P(tableNameZval) || Z_STRLEN_P(tableNameZval) <= 0){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord] method to realize the tableName () abstract methods and return to the operation of the database table name", 9001 TSRMLS_CC);
			return;
		}
		tableName = estrdup(Z_STRVAL_P(tableNameZval));
		zval_dtor(&callReturnTemp);
	MODULE_END

	//��ȡ�����ļ�
	MODULE_BEGIN
		zval	*dbSlaveConfigs,
				**tablePreZval;

		CConfig_load("DB.main.master",cconfigInstanceZval,&dbSlaveConfigs TSRMLS_CC);
		if(IS_ARRAY != Z_TYPE_P(dbSlaveConfigs)){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord] Failed to get the configuration[DB->main->master]", 9001 TSRMLS_CC);
			return;
		}
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(dbSlaveConfigs),"tablePrefix",strlen("tablePrefix")+1,(void**)&tablePreZval) && IS_STRING == Z_TYPE_PP(tablePreZval)){
			tablePreFix = estrdup(Z_STRVAL_PP(tablePreZval));
		}else{
			tablePreFix = estrdup("");
		}
		zval_ptr_dtor(&dbSlaveConfigs);
	MODULE_END

	//��������
	strcat2(&tableAllName,tablePreFix,tableName,NULL);


	//CDatabase::getInstance����
	MODULE_BEGIN
		CDatabase_getInstance(0,"main",&CDatabase TSRMLS_CC);
	MODULE_END


	conditionZval = zend_read_property(CActiveRecordCe,getThis(),ZEND_STRL("condition"), 0 TSRMLS_CC);
	paramsZval = zend_read_property(CActiveRecordCe,getThis(),ZEND_STRL("params"), 0 TSRMLS_CC);
	whereZval = zend_read_property(CActiveRecordCe,getThis(),ZEND_STRL("where"), 0 TSRMLS_CC);	

	if(IS_ARRAY != Z_TYPE_P(paramsZval)){
		MAKE_STD_ZVAL(paramsZval);
		array_init(paramsZval);
	}

	if(Z_TYPE_P(conditionZval) != IS_NULL && IS_STRING == Z_TYPE_P(conditionZval) && strlen(Z_STRVAL_P(conditionZval)) > 0 ){

		char *updateSql,
			 *updateWhere,
			 *updateSendSQL;

		int i = 0,j = 0;

		char	*key,
				*saveVal,
				keyName[10240];
		ulong	ukey;

		zval	**thisVal,
				*bindParams,
				*saveParams;

		smart_str smart_updateStr = {0};

		CActiveRecord_getAllSetAttribute(getThis(),&publicAttributeList TSRMLS_CC);

		if(!IS_ARRAY == Z_TYPE_P(publicAttributeList) ){
			zend_throw_exception(CDbExceptionCe, "[ActiveRecordException] Call CActiveRecord->save (), no attribute can be updated", 5010 TSRMLS_CC);
			return;
		}

		//�����������SQL
		j = zend_hash_num_elements(Z_ARRVAL_P(publicAttributeList));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(publicAttributeList));
		for(i = 0; i < j ; i++){

			zend_hash_get_current_key(Z_ARRVAL_P(publicAttributeList),&key,&ukey,0);
			zend_hash_get_current_data(Z_ARRVAL_P(publicAttributeList),(void**)&thisVal);

			if(IS_STRING != Z_TYPE_PP(thisVal) && IS_LONG != Z_TYPE_PP(thisVal) && IS_DOUBLE != Z_TYPE_PP(thisVal)){
				zend_hash_move_forward(Z_ARRVAL_P(publicAttributeList));
				continue;
			}

			smart_str_appends(&smart_updateStr,"`");
			smart_str_appends(&smart_updateStr,key);
			smart_str_appends(&smart_updateStr,"` = :");
			smart_str_appends(&smart_updateStr,key);

			MAKE_STD_ZVAL(saveParams);
			ZVAL_ZVAL(saveParams,*thisVal,1,0);
			sprintf(keyName,"%s%s",":",key);
			add_assoc_zval(paramsZval,keyName,saveParams);

			if(i < j - 1){
				smart_str_appends(&smart_updateStr," ,");
			}

			zend_hash_move_forward(Z_ARRVAL_P(publicAttributeList));
		}

		smart_str_0(&smart_updateStr);

		//ƴװSQL
		strcat2(&updateSql,"UPDATE `",tableAllName,"` SET ",smart_updateStr.c," WHERE ",Z_STRVAL_P(conditionZval),NULL);
		smart_str_free(&smart_updateStr);

		//ִ��prepare
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"prepare",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],updateSql,1);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		efree(updateSql);

		//ִ��execute
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"execute",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],paramsZval,1,1);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);

		zval_dtor(&callReturn);
		zval_ptr_dtor(&publicAttributeList);

	}else if(IS_ARRAY == Z_TYPE_P(whereZval) && zend_hash_num_elements(Z_ARRVAL_P(whereZval)) > 0){

		//ִ��update
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"update",0);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);

		//ִ��from
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"from",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],tableName,1);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);

		//���б�����ֵ
		CActiveRecord_getAllSetAttribute(getThis(),&publicAttributeList TSRMLS_CC);

		//value
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"value",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],publicAttributeList,1,0);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);

		//where
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"where",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],whereZval,1,0);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);

		//ִ��execute
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"execute",0);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);
		
		zval_ptr_dtor(&publicAttributeList);
		zval_dtor(&callReturn);

	}else{

		//ִ��insert
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"insert",0);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);
	
		//ִ��from
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"from",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],tableName,1);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);

		//���б�����ֵ
		CActiveRecord_getAllSetAttribute(getThis(),&publicAttributeList TSRMLS_CC);

		//value
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"value",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],publicAttributeList,1,0);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);

		//ִ��execute
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"execute",0);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_dtor(&callReturn);
		zval_ptr_dtor(&publicAttributeList);

		//�ж��Ƿ����쳣
		if(EG(exception)){

			//ȷ���쳣���Ƿ�ΪPDOException
			zend_class_entry *exceptionCe;
			zval *exceptionMessage;

			exceptionCe = Z_OBJCE_P(EG(exception));
			exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);

			//��¼������Ĵ���
			zend_update_property_string(CActiveRecordCe,getThis(), ZEND_STRL("_lastError"),Z_STRVAL_P(exceptionMessage) TSRMLS_CC);
			zend_clear_exception(TSRMLS_C);
			zval_ptr_dtor(&cconfigInstanceZval);
			zval_ptr_dtor(&CDatabase);
			efree(tableAllName);
			efree(tablePreFix);
			efree(tableName);
			RETURN_FALSE;
		}

	}

	//����
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&CDatabase);
	efree(tableAllName);
	efree(tablePreFix);
	efree(tableName);
	RETVAL_TRUE;

}

PHP_METHOD(CActiveRecord,add)
{
	zval *tableNameZval,
		 *configZval,
		 *cconfigInstanceZval,
		 *CDatabase,
		 callAction,
		 callReturn,
		 *paramsList[3],
		 param1,
		 param2,
		 param3,
		 *functionReturn,
		 *saveArZval,
		 *funcCopy,
		 *saveCopy;

	char *tableName = "",
		 *tablePreFix = "",
		 *tableAllName = "",
		 *sql = "",
		 tempTableName[10240];

	zval *data;

	paramsList[0] = &param1;
	paramsList[1] = &param2;
	paramsList[2] = &param3;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&data) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord->add] Parameter error", 9001 TSRMLS_CC);
		return;
	}

	//���ö���
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//��ȡ����
	MODULE_BEGIN
		zval	callAction,
				callReturn;

		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"tableName",0);
		call_user_function(NULL, &getThis(), &callAction, &callReturn,0, NULL TSRMLS_CC);
		tableNameZval = &callReturn;
		if(IS_STRING != Z_TYPE_P(tableNameZval) || strlen(Z_STRVAL_P(tableNameZval)) <= 0){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord] method to realize the tableName () abstract methods and return to the operation of the database table name", 9001 TSRMLS_CC);
			return;
		}
		tableName = estrdup(Z_STRVAL_P(tableNameZval));
		zval_dtor(&callReturn);
	MODULE_END


	//CDatabase::getInstance����
	MODULE_BEGIN
		CDatabase_getInstance(0,"main",&CDatabase TSRMLS_CC);
	MODULE_END

	//ִ��insert
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"insert",0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);

	//ִ��from
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"from",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_STRING(paramsList[0],tableName,1);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
	zval_ptr_dtor(&paramsList[0]);
	efree(tableName);

	//ִ��value
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"value",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_ZVAL(paramsList[0],data,1,0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
	zval_ptr_dtor(&paramsList[0]);

	//ִ��execute
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"execute",0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);

	//�ж��Ƿ񱨸��쳣
	if(EG(exception)){

		//ȷ���쳣���Ƿ�ΪPDOException
		zend_class_entry *exceptionCe;
		zval *exceptionMessage;

		exceptionCe = Z_OBJCE_P(EG(exception));
		exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);

		//��¼������Ĵ���
		zend_update_property_string(CActiveRecordCe,getThis(), ZEND_STRL("_lastError"),Z_STRVAL_P(exceptionMessage) TSRMLS_CC);
		zend_clear_exception(TSRMLS_C);
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&CDatabase);
		zval_dtor(&callReturn);
		RETURN_FALSE;
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&CDatabase);
	zval_dtor(&callReturn);

	RETVAL_TRUE;
}

PHP_METHOD(CActiveRecord,count)
{
	zval *tableNameZval,
		 *configZval,
		 *cconfigInstanceZval,
		 *CDatabase,
		 callAction,
		 callReturn,
		 *paramsList[3],
		 param1,
		 param2,
		 param3,
		 *functionReturn,
		 *funcCopy,
		 *saveArZval,
		 *saveCopy;

	char *tableName = "";

	//����������
	zval *where = NULL;

	paramsList[0] = &param1;
	paramsList[1] = &param2;
	paramsList[2] = &param3;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|z",&where) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord->find] Parameter error", 9001 TSRMLS_CC);
		return;
	}

	//��ȡ����
	MODULE_BEGIN
		zval	callAction,
				callReturn;

		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"tableName",0);
		call_user_function(NULL, &getThis(), &callAction, &callReturn,0, NULL TSRMLS_CC);
		tableNameZval = &callReturn;
		if(IS_STRING != Z_TYPE_P(tableNameZval) || strlen(Z_STRVAL_P(tableNameZval)) <= 0){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord] method to realize the tableName () abstract methods and return to the operation of the database table name", 9001 TSRMLS_CC);
			return;
		}
		tableName = estrdup(Z_STRVAL_P(tableNameZval));
		zval_dtor(&callReturn);
	MODULE_END

	//CDatabase::getInstance����
	MODULE_BEGIN
		CDatabase_getInstance(0,"main",&CDatabase TSRMLS_CC);
	MODULE_END

	//ִ��select
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"select",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_STRING(paramsList[0],"COUNT(*)",1);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
	zval_ptr_dtor(&paramsList[0]);

	//ִ��from
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"from",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_STRING(paramsList[0],tableName,1);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
	zval_ptr_dtor(&paramsList[0]);
	efree(tableName);

	//where
	if(where != NULL && Z_TYPE_P(where) != IS_NULL && IS_ARRAY == Z_TYPE_P(where) && zend_hash_num_elements(Z_ARRVAL_P(where)) >0 ){
		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"where",0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],where,1,0);
		call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
	}

	//ִ��execute
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"execute",0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);
	functionReturn = &callReturn;


	//�ж������쳣
	if(EG(exception)){
		zend_class_entry *exceptionCe;
		zval *exceptionMessage;
		exceptionCe = Z_OBJCE_P(EG(exception));
		exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);

		//��¼������Ĵ���
		zend_update_property_string(CActiveRecordCe,getThis(), ZEND_STRL("_lastError"),Z_STRVAL_P(exceptionMessage) TSRMLS_CC);
		zend_clear_exception(TSRMLS_C);

		zval_ptr_dtor(&CDatabase);
		zval_dtor(&callReturn);

		RETVAL_LONG(0);
		return;
	}

	//��ȡCOUNT(*)����
	MODULE_BEGIN
		zval	**countZval,
				currentReturn;

		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"current",0);
		call_user_function(NULL, &functionReturn, &callAction, &currentReturn, 0, NULL TSRMLS_CC);
		saveCopy = &currentReturn;

		if(IS_ARRAY == Z_TYPE_P(saveCopy) && zend_hash_find(Z_ARRVAL_P(saveCopy),"COUNT(*)",strlen("COUNT(*)")+1,(void**)&countZval) == SUCCESS){
			RETVAL_ZVAL(*countZval,1,0);
		}
		zval_dtor(&currentReturn);
	MODULE_END


	zval_ptr_dtor(&CDatabase);
	zval_dtor(&callReturn);
}

PHP_METHOD(CActiveRecord,update)
{
	zval *tableNameZval,
		 *configZval,
		 *cconfigInstanceZval,
		 *CDatabase,
		 callAction,
		 callReturn,
		 *paramsList[3],
		 param1,
		 param2,
		 param3,
		 *functionReturn,
		 *saveArZval,
		 *funcCopy,
		 *saveCopy;

	char *tableName = "";

	zval *data,
		 *where;

	paramsList[0] = &param1;
	paramsList[1] = &param2;
	paramsList[2] = &param3;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zz",&data,&where) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord->add] Parameter error", 9001 TSRMLS_CC);
		return;
	}

	//��ȡ����
	MODULE_BEGIN
		zval	callAction,
				callReturn;

		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"tableName",0);
		call_user_function(NULL, &getThis(), &callAction, &callReturn,0, NULL TSRMLS_CC);
		tableNameZval = &callReturn;
		if(IS_STRING != Z_TYPE_P(tableNameZval) || strlen(Z_STRVAL_P(tableNameZval)) <= 0){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord] method to realize the tableName () abstract methods and return to the operation of the database table name", 9001 TSRMLS_CC);
			return;
		}
		tableName = estrdup(Z_STRVAL_P(tableNameZval));
		zval_dtor(&callReturn);
	MODULE_END

	//CDatabase::getInstance����
	MODULE_BEGIN
		CDatabase_getInstance(0,"main",&CDatabase TSRMLS_CC);
	MODULE_END

	//ִ��update
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"update",0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);

	//ִ��from
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"from",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_STRING(paramsList[0],tableName,1);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
	zval_ptr_dtor(&paramsList[0]);

	//ִ��value
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"value",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_ZVAL(paramsList[0],data,1,0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
	zval_ptr_dtor(&paramsList[0]);

	//ִ��where
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"where",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_ZVAL(paramsList[0],where,1,0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
	zval_ptr_dtor(&paramsList[0]);

	//ִ��execute
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"execute",0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);

	efree(tableName);
	zval_ptr_dtor(&CDatabase);
	zval_dtor(&callReturn);

	//�ж��Ƿ�����쳣
	if(EG(exception)){
		zend_class_entry *exceptionCe;
		zval *exceptionMessage;
		exceptionCe = Z_OBJCE_P(EG(exception));
		exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);

		//��¼������Ĵ���
		zend_update_property_string(CActiveRecordCe,getThis(), ZEND_STRL("_lastError"),Z_STRVAL_P(exceptionMessage) TSRMLS_CC);
		zend_clear_exception(TSRMLS_C);
		RETURN_FALSE;
	}


	RETVAL_TRUE;
}

PHP_METHOD(CActiveRecord,delete)
{
	zval *tableNameZval,
		 *configZval,
		 *cconfigInstanceZval,
		 *CDatabase,
		 callAction,
		 callReturn,
		 *paramsList[3],
		 param1,
		 param2,
		 param3,
		 *functionReturn,
		 *saveArZval,
		 *funcCopy,
		 *saveCopy;

	char *tableName = "";

	zval *where;

	paramsList[0] = &param1;
	paramsList[1] = &param2;
	paramsList[2] = &param3;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&where) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord->delete] Parameter error,must given an array", 9001 TSRMLS_CC);
		return;
	}

	//��ȡ����
	MODULE_BEGIN
		zval	callAction,
				callReturn;

		INIT_ZVAL(callAction);
		ZVAL_STRING(&callAction,"tableName",0);
		call_user_function(NULL, &getThis(), &callAction, &callReturn,0, NULL TSRMLS_CC);
		tableNameZval = &callReturn;
		if(IS_STRING != Z_TYPE_P(tableNameZval) || Z_STRLEN_P(tableNameZval) <= 0){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CActiveRecord] method to realize the tableName () abstract methods and return to the operation of the database table name", 9001 TSRMLS_CC);
			return;
		}
		tableName = estrdup(Z_STRVAL_P(tableNameZval));
		zval_dtor(&callReturn);
	MODULE_END

	//CDatabase::getInstance����
	MODULE_BEGIN
		CDatabase_getInstance(0,"main",&CDatabase TSRMLS_CC);
	MODULE_END

	//ִ��delete
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"delete",0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);

	//ִ��from
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"from",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_STRING(paramsList[0],tableName,1);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
	zval_ptr_dtor(&paramsList[0]);

	//ִ��where
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"where",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_ZVAL(paramsList[0],where,1,0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 1, paramsList TSRMLS_CC);
	zval_ptr_dtor(&paramsList[0]);

	//ִ��execute
	INIT_ZVAL(callAction);
	ZVAL_STRING(&callAction,"execute",0);
	call_user_function(NULL, &CDatabase, &callAction, &callReturn, 0, NULL TSRMLS_CC);

	efree(tableName);
	zval_ptr_dtor(&CDatabase);
	zval_dtor(&callReturn);

	//�ж��Ƿ�����쳣
	if(EG(exception)){
		zend_class_entry *exceptionCe;
		zval *exceptionMessage;
		exceptionCe = Z_OBJCE_P(EG(exception));
		exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);

		//��¼������Ĵ���
		zend_update_property_string(CActiveRecordCe,getThis(), ZEND_STRL("_lastError"),Z_STRVAL_P(exceptionMessage) TSRMLS_CC);
		zend_clear_exception(TSRMLS_C);
		RETURN_FALSE;
	}


	RETVAL_TRUE;
}

PHP_METHOD(CActiveRecord,getLastError)
{
	zval *lastError;
	lastError = zend_read_property(CActiveRecordCe,getThis(), ZEND_STRL("_lastError"),0 TSRMLS_CC);
	RETVAL_ZVAL(lastError,1,0);
}



PHP_METHOD(CActiveRecord,tableName)
{

}
