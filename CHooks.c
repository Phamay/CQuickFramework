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
#include "php_CHooks.h"
#include "php_CException.h"
#include "php_CWebApp.h"
#include "php_CDebug.h"


//zend�෽��
zend_function_entry CHooks_functions[] = {
	PHP_ME(CHooks,loadPlugin,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,_getPathFile,NULL,ZEND_ACC_PRIVATE | ZEND_ACC_STATIC)
	PHP_ME(CHooks,getHooksRegisterList,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,getPluginLoadSuccess,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,getPluginLoadFail,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,callHooks,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,getHooksList,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,_setHooksFunctionLevel,NULL,ZEND_ACC_PRIVATE | ZEND_ACC_STATIC)
	PHP_ME(CHooks,registerHook,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,__destruct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	{NULL, NULL, NULL}
};

//ģ�鱻����ʱ
CMYFRAME_REGISTER_CLASS_RUN(CHooks)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CHooks",CHooks_functions);
	CHooksCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//�������_pluginList _hooks _failLoadPluginList
	zend_declare_property_null(CHooksCe, ZEND_STRL("_pluginList"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CHooksCe, ZEND_STRL("_hooks"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CHooksCe, ZEND_STRL("_failLoadPluginList"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CHooksCe, ZEND_STRL("_allHooks"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS;
}

zend_function_entry CDataObject_functions[] = {
	PHP_ME(CDataObject,asArray,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDataObject,set,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDataObject,get,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//CDataObject
CMYFRAME_REGISTER_CLASS_RUN(CDataObject)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CDataObject",CDataObject_functions);
	CDataObjectCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//�������_pluginList _hooks _failLoadPluginList
	zend_declare_property_null(CDataObjectCe, ZEND_STRL("data"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}


//��ȡĿ¼�µ��ļ�
int CHooks_getPathFile(char *path,zval **returnZval TSRMLS_DC)
{
	zval	*dirFile;

	MAKE_STD_ZVAL(*returnZval);
	array_init(*returnZval);

	//�ж�Ŀ¼����
	if(SUCCESS != fileExist(path)){
		return FAILURE;
	}

	//��ȡĿ¼
	php_scandir(path,&dirFile);

	//����Ŀ¼ ȥ��������ŵ�
	if(IS_ARRAY == Z_TYPE_P(dirFile)){

		int		i,
				dirNum;

		zval	**nData,
				*filterArray,
				*fileParams;


		MAKE_STD_ZVAL(filterArray);
		array_init(filterArray);

		

		//������Ҫ����Ŀ¼
		add_next_index_string(filterArray,".",1);
		add_next_index_string(filterArray,"..",1);
		add_next_index_string(filterArray,".svn",1);

		//����ȡ����Ŀ¼
		dirNum = zend_hash_num_elements(Z_ARRVAL_P(dirFile));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(dirFile));
		for(i = 0 ; i < dirNum ; i++){
			zend_hash_get_current_data(Z_ARRVAL_P(dirFile),(void**)&nData);

			if(0 == in_array(Z_STRVAL_PP(nData),filterArray)){
				add_next_index_string(*returnZval,Z_STRVAL_PP(nData),1);
			}

			zend_hash_move_forward(Z_ARRVAL_P(dirFile));
		}

		//����
		zval_ptr_dtor(&dirFile);
		zval_ptr_dtor(&filterArray);
		return SUCCESS;
	}

	zval_ptr_dtor(&dirFile);
	return FAILURE;
}

void CHooks_loadSystemPlugin(TSRMLS_D){

	zval	*pluginObject;

	MAKE_STD_ZVAL(pluginObject);
	object_init_ex(pluginObject,CDebugCe);

	//�����乹�캯��
	if (CDebugCe->constructor) {
		zval constructReturn;
		zval constructVal;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, CDebugCe->constructor->common.function_name, 0);
		call_user_function(NULL, &pluginObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);
	}

	//����setHooksע�ắ��
	MODULE_BEGIN
		zval constructReturn;
		zval constructVal;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, "setHooks", 0);
		call_user_function(NULL, &pluginObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);
	MODULE_END

	//save to instance static
	zend_update_static_property(CDebugCe,ZEND_STRL("instance"),pluginObject TSRMLS_CC);

	zval_ptr_dtor(&pluginObject);
}

//������
void CHooks_loadPlugin(TSRMLS_D){
	
	zval	*returnZval,
			*cconfigInstanceZval,
			*loadPluginZval,
			*loadPluginListZval,
			*loadPluginPathConfZval,
			*codePath,
			*appPath,
			*pluginPathZval;

	char	*pluginPath;

	zend_class_entry **cwebClassEntry;

	zend_hash_find(EG(class_table),"cwebapp",strlen("cwebapp")+1,(void**)&cwebClassEntry);
	codePath = zend_read_static_property(*cwebClassEntry,ZEND_STRL("code_path"),0 TSRMLS_CC);
	appPath = zend_read_static_property(*cwebClassEntry,ZEND_STRL("app_path"),0 TSRMLS_CC);

	//��ȡLOAD_PLUGIN��LOAD_LIST��������
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("LOAD_PLUGIN",cconfigInstanceZval,&loadPluginZval TSRMLS_CC);

	CConfig_load("LOAD_LIST",cconfigInstanceZval,&loadPluginListZval TSRMLS_CC);
	CConfig_load("PLUGIN_PATH",cconfigInstanceZval,&loadPluginPathConfZval TSRMLS_CC);

	if(IS_BOOL != Z_TYPE_P(loadPluginZval)){
		zval_ptr_dtor(&loadPluginZval);
		zval_ptr_dtor(&loadPluginListZval);
		zval_ptr_dtor(&loadPluginPathConfZval);
		zval_ptr_dtor(&cconfigInstanceZval);
		return;
	}

	//�����ز��
	if(IS_BOOL == Z_TYPE_P(loadPluginZval) && Z_LVAL_P(loadPluginZval) == 0){
		zval_ptr_dtor(&loadPluginZval);
		zval_ptr_dtor(&loadPluginListZval);
		zval_ptr_dtor(&loadPluginPathConfZval);
		zval_ptr_dtor(&cconfigInstanceZval);
		return;
	}


	//���·��
	if(IS_STRING == Z_TYPE_P(loadPluginPathConfZval) && Z_STRLEN_P(loadPluginPathConfZval) > 0 ){
		strcat2(&pluginPath,Z_STRVAL_P(appPath),"/",Z_STRVAL_P(loadPluginPathConfZval),"/",NULL);
	}else{
		strcat2(&pluginPath,Z_STRVAL_P(appPath),"/plugins/",NULL);
	}
	zval_ptr_dtor(&loadPluginPathConfZval);


	//������·������
	zend_update_static_property_string(CWebAppCe,ZEND_STRL("cplugin_path"), pluginPath TSRMLS_CC);
	pluginPathZval = zend_read_static_property(CWebAppCe,ZEND_STRL("cplugin_path"),0 TSRMLS_CC);


	//����ȫ�����
	if(IS_NULL == Z_TYPE_P(loadPluginListZval)){
		
		zval	*pluginDirList,
				**pluginNameZval,
				*loadFailList,
				*loadSuccessList;

		int		pluginNum,
				i,
				loadFileStatus;

		char	pluginConfigFileName[1024];


		MAKE_STD_ZVAL(loadSuccessList);
		array_init(loadSuccessList);

		MAKE_STD_ZVAL(loadFailList);
		array_init(loadFailList);

		//��ȡĿ¼�µ����в���ļ���
		CHooks_getPathFile(pluginPath,&pluginDirList TSRMLS_CC);

		if(IS_ARRAY != Z_TYPE_P(pluginDirList)){
			zval_ptr_dtor(&loadFailList);
			zval_ptr_dtor(&loadSuccessList);
			return;
		}

		//���ζ�ȡ���
		pluginNum = zend_hash_num_elements(Z_ARRVAL_P(pluginDirList));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(pluginDirList));
		for(i = 0 ; i < pluginNum ; i++)
		{
			zend_hash_get_current_data(Z_ARRVAL_P(pluginDirList),(void**)&pluginNameZval);

			//��ȡ������������ļ�
			sprintf(pluginConfigFileName,"%s%s%s%s%s",Z_STRVAL_P(pluginPathZval),Z_STRVAL_PP(pluginNameZval),"/",Z_STRVAL_PP(pluginNameZval),".php");

			//�����ļ�������
			if(SUCCESS != fileExist(pluginConfigFileName)){
				//���˼������ʧ���б�
				zval	*thisFailData;
				add_next_index_string(loadFailList,Z_STRVAL_PP(pluginNameZval),1);
				zend_hash_move_forward(Z_ARRVAL_P(pluginDirList));
				continue;
			}

			//���Լ��ز���������ļ� 
			loadFileStatus = CLoader_loadFile(pluginConfigFileName);

			//����ʵ�����ò���ļ�
			if(SUCCESS == loadFileStatus){
				zend_class_entry	**thisClass,
									*thisClassP;

				char *className;

				zval *pluginObject;

				//����
				className = estrdup(Z_STRVAL_PP(pluginNameZval));
				php_strtolower(className,strlen(className)+1);

				//������
				if(zend_hash_find(EG(class_table),className,strlen(className)+1,(void**)&thisClass ) == FAILURE){
					//����Ҳ������򷢳�һ������
					php_error_docref(NULL TSRMLS_CC,E_NOTICE,"Plugin[%s] lose base define Class",className);
					zend_hash_move_forward(Z_ARRVAL_P(pluginDirList));
					efree(className);
					continue;
				}

				//ȡ��ַ
				thisClassP = *thisClass;
				
				//����Ƿ�̳�CPlugin
				if(thisClassP->parent){
					char *parentName;
					parentName = estrdup(thisClassP->parent->name);
					php_strtolower(parentName,strlen(parentName)+1);
					if(strcmp(parentName,"cplugin") != 0){
						php_error_docref(NULL TSRMLS_CC,E_NOTICE,"Plugin[%s] must extends the class CPlugin",Z_STRVAL_PP(pluginNameZval));
						zend_hash_move_forward(Z_ARRVAL_P(pluginDirList));
						//�������ʧ���б�
						add_next_index_string(loadFailList,Z_STRVAL_PP(pluginNameZval),1);
						efree(className);
						continue;
					}
					efree(parentName);
				}else{
					php_error_docref(NULL TSRMLS_CC,E_NOTICE,"Plugin[%s] must extends the class CPlugin",Z_STRVAL_PP(pluginNameZval));
					zend_hash_move_forward(Z_ARRVAL_P(pluginDirList));
					//�������ʧ���б�
					add_next_index_string(loadFailList,Z_STRVAL_PP(pluginNameZval),1);
					efree(className);
					continue;
				}

				//ʵ�����ò��
				MAKE_STD_ZVAL(pluginObject);
				object_init_ex(pluginObject,thisClassP);

				//�����乹�캯��
				if (thisClassP->constructor) {
					zval constructReturn;
					zval constructVal;
					INIT_ZVAL(constructVal);
					ZVAL_STRING(&constructVal, thisClassP->constructor->common.function_name, 0);
					call_user_function(NULL, &pluginObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
					zval_dtor(&constructReturn);
				}

				//����setHooksע�ắ��
				MODULE_BEGIN
					zval constructReturn;
					zval constructVal;
					INIT_ZVAL(constructVal);
					ZVAL_STRING(&constructVal, "setHooks", 0);
					call_user_function(NULL, &pluginObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
					zval_dtor(&constructReturn);
				MODULE_END

				zval_ptr_dtor(&pluginObject);

				//�������������б�
				add_next_index_string(loadSuccessList,Z_STRVAL_PP(pluginNameZval),1);


				efree(className);
			}

			zend_hash_move_forward(Z_ARRVAL_P(pluginDirList));
		}

		//���ɹ���ʧ�ܵĸ����������
		zend_update_static_property(CHooksCe,ZEND_STRL("_pluginList"),loadSuccessList TSRMLS_CC);
		zend_update_static_property(CHooksCe,ZEND_STRL("_failLoadPluginList"),loadFailList TSRMLS_CC);

		zval_ptr_dtor(&pluginDirList);
		zval_ptr_dtor(&loadFailList);
		zval_ptr_dtor(&loadSuccessList);
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&loadPluginZval);
	zval_ptr_dtor(&loadPluginListZval);

	efree(pluginPath);

	//ȫ�޵��������ʧ��
	return;
}

//��������
PHP_METHOD(CHooks,__destruct){

}


//�෽��:����Ӧ�ö���
PHP_METHOD(CHooks,loadPlugin)
{
	CHooks_loadPlugin(TSRMLS_C);
	RETVAL_LONG(1);
}


//��ȡĿ¼�µ��ļ�
PHP_METHOD(CHooks,_getPathFile)
{
	char	*path;
	int		pathLen;

	zval	*returnZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&path,&pathLen) == FAILURE){
		return;
	}

	CHooks_getPathFile(path,&returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,1);
}

//��ȡ��ע��Ĺ����б�
PHP_METHOD(CHooks,getHooksRegisterList)
{
	//����_hooks����
	zval *hooksZval;

	hooksZval = zend_read_static_property(CHooksCe,ZEND_STRL("_hooks"), 0 TSRMLS_CC);
	ZVAL_ZVAL(return_value,hooksZval,1,0);
}

//��ȡ��ע��Ĳ���б�
PHP_METHOD(CHooks,getPluginLoadSuccess)
{
	//����_pluginList����
	zval *hooksZval;

	hooksZval = zend_read_static_property(CHooksCe,ZEND_STRL("_pluginList"), 0 TSRMLS_CC);

	ZVAL_ZVAL(return_value,hooksZval,1,0);
}

//����Hooks�ĵ��õȼ�
void CHooks_setHooksFunctionLevel(zval *functionList,zval **levelList TSRMLS_DC){

	zval **table1Zval,
		 **table2Zval,
		 *addKey,
		 *saveSortZval;
	HashTable *sortArr;
	int	i,
		j,
		num1,
		num2;
	char *key1,
		 *key2,
		 *addKeyStr;
	ulong ukey1,
		  ukey2;

	ALLOC_HASHTABLE(sortArr);
	zend_hash_init(sortArr,8,NULL,NULL,0);

	num1 = zend_hash_num_elements(Z_ARRVAL_P(functionList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(functionList));
	for(i = 0 ; i < num1 ;i++){
		zend_hash_get_current_key(Z_ARRVAL_P(functionList),&key1,&ukey1,0);
		zend_hash_get_current_data(Z_ARRVAL_P(functionList),(void**)&table1Zval);
		
		num2 = zend_hash_num_elements(Z_ARRVAL_PP(table1Zval));
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(table1Zval));
		for(j = 0 ; j < num2 ; j++){	
			zend_hash_get_current_key(Z_ARRVAL_PP(table1Zval),&key2,&ukey2,0);
			zend_hash_get_current_data(Z_ARRVAL_PP(table1Zval),(void**)&table2Zval);
			
			//����һ��keyֵ
			strcat2(&addKeyStr,key1,"|",key2,NULL);
			add_assoc_string(*table2Zval,"key",addKeyStr,1);
			efree(addKeyStr);

			//��sortArr���ֵ
			zend_hash_next_index_insert(sortArr,&*table2Zval,sizeof(zval*),NULL);

			zend_hash_move_forward(Z_ARRVAL_PP(table1Zval));
		}


		zend_hash_move_forward(Z_ARRVAL_P(functionList));
	}

	MAKE_STD_ZVAL(saveSortZval);
	Z_TYPE_P(saveSortZval) = IS_ARRAY;
	Z_ARRVAL_P(saveSortZval) = sortArr;
	CArraySort_sortArrayDesc(saveSortZval,"callLevel",&*levelList TSRMLS_CC);
	zval_ptr_dtor(&saveSortZval);
}

//��ȡע��ʧ�ܵ� 
PHP_METHOD(CHooks,getPluginLoadFail)
{
	//����_failLoadPluginList����
	zval *hooksZval;

	hooksZval = zend_read_static_property(CHooksCe,ZEND_STRL("_failLoadPluginList"), 0 TSRMLS_CC);
	ZVAL_ZVAL(return_value,hooksZval,1,0);
}

//����ע�ᵽHooks�ĺ���
void CHooks_callHooks(char *hookName,zval* paramList[],zend_uint paramNum TSRMLS_DC){

	int n,
		funNum;

	zval *hooksList,
		 *hooksNameZval,
		 **thisHooksFunctionList,
		 *callFunctionSort,
		 **callFunctionData,
		 **keyData,
		 **callObject,
		 **callNum,
		 hookCallAction,
		 hookCallRetrun;

	char *hooksKeyName = NULL,
		 *pluginName = NULL,
		 *functionName = NULL;

	zend_function *callFunc;

	//��ȡ����Hooks����
	hooksList = zend_read_static_property(CHooksCe,ZEND_STRL("_hooks"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(hooksList)){
		return;
	}

	//��ȡ��Hooks�µĺ���
	if(zend_hash_find(Z_ARRVAL_P(hooksList),hookName,strlen(hookName)+1,(void**)&thisHooksFunctionList) == FAILURE){
		return;
	}

	//����call_level���¶Դ����ú�����������
	CHooks_setHooksFunctionLevel(*thisHooksFunctionList,&callFunctionSort TSRMLS_CC);

	//���������ú����б�
	funNum = zend_hash_num_elements(Z_ARRVAL_P(callFunctionSort));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(callFunctionSort));
	for(n = 0 ; n < funNum; n++){
		zend_hash_get_current_data(Z_ARRVAL_P(callFunctionSort),(void**)&callFunctionData);
		zend_hash_find(Z_ARRVAL_PP(callFunctionData),"key",strlen("key")+1,(void**)&keyData);

		hooksKeyName = estrdup(Z_STRVAL_PP(keyData));
		pluginName = strtok(hooksKeyName,"|");
		functionName = strtok(NULL,"|");
		if(pluginName == NULL || functionName == NULL){
			zend_hash_move_forward(Z_ARRVAL_P(callFunctionSort));
			efree(hooksKeyName);
			continue;
		}

		//�ж��ṩ���õĶ�����ȷ callObject
		zend_hash_find(Z_ARRVAL_PP(callFunctionData),"callObject",strlen("callObject")+1,(void**)&callObject);
		if(IS_OBJECT != Z_TYPE_PP(callObject)){
			zend_hash_move_forward(Z_ARRVAL_P(callFunctionSort));
			efree(hooksKeyName);
			continue;
		}

		//���ô���
		zend_hash_find(Z_ARRVAL_PP(callFunctionData),"callNum",strlen("callNum")+1,(void**)&callNum);

		//�жϸö������Ƿ���ڸ÷���
		php_strtolower(functionName,strlen(functionName)+1);

		if( ! zend_hash_exists(& (Z_OBJCE_PP(callObject)->function_table),functionName,strlen(functionName)+1) ){
			zend_hash_move_forward(Z_ARRVAL_P(callFunctionSort));
			efree(hooksKeyName);
			continue;
		}

		//���øú���
		INIT_ZVAL(hookCallAction);
		ZVAL_STRING(&hookCallAction,functionName,0);

		//���ù��Ӻ���
		if(SUCCESS == call_user_function(NULL,callObject,&hookCallAction,&hookCallRetrun, paramNum ,paramList TSRMLS_CC)){
			int updateCallNum = 1;
			zval *updateCallNumZval;
			if(IS_LONG == Z_TYPE_PP(callNum)){
				Z_LVAL_PP(callNum) = Z_LVAL_PP(callNum) + 1;
			}
		}

		zval_dtor(&hookCallRetrun);
		zend_hash_move_forward(Z_ARRVAL_P(callFunctionSort));
		efree(hooksKeyName);
	}

	zval_ptr_dtor(&callFunctionSort);
}

//����ע�ᵽHooks�ĺ���
PHP_METHOD(CHooks,callHooks)
{
	int argc = ZEND_NUM_ARGS(),
		i,
		n,
		funNum;

	zval ***args,
		 *paramList[32],
		 thisVal;

	char *hookName = NULL;

	args = (zval***)safe_emalloc(argc,sizeof(zval**),0);

	if(ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc,args) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[HooksException] Call the Hooks function when you need to specify the name of the hook", 7001 TSRMLS_CC);
		efree(args);
		return;
	}else{
		//����������Ŀ����
		for(i = 0 ; i < argc ; i++){
			if(i == 0){
				//���õ�Hooks����
				if(IS_STRING != Z_TYPE_PP(args[i])){
					continue;
				}
				hookName = Z_STRVAL_PP(args[i]);
			}else{
				//���λ�ȡ��Ҫ���ݵĲ���
				MAKE_STD_ZVAL(paramList[i-1]);
				ZVAL_ZVAL(paramList[i-1],*args[i],1,0);
			}
		}
	}

	CHooks_callHooks(hookName,paramList,i-1 TSRMLS_CC);
	efree(args);
	for(n = 0 ; n < i-1;n++){
		zval_ptr_dtor(&paramList[n]);
	}
}

PHP_METHOD(CHooks,getHooksList)
{
	zval *allHookList;
	allHookList = zend_read_static_property(CHooksCe,ZEND_STRL("_allHooks"),0 TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(allHookList)){
		RETURN_ZVAL(allHookList,1,0);
	}


	//���ɲ���б�
	MAKE_STD_ZVAL(allHookList);
	array_init(allHookList);

	//��¼Hooks
	add_assoc_string(allHookList,"HOOKS_ROUTE_START","Framework parse URL request before triggering this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_ROUTE_END","Framework parse URL request is completed to trigger this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_ROUTE_ERROR","Frameword unable to route request will triggered this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_CONTROLLER_INIT","Framework trigger this Hooks after init controller",0);
	add_assoc_string(allHookList,"HOOKS_ACTION_INIT","Framework triggering this Hooks after action has run",0);
	add_assoc_string(allHookList,"HOOKS_EXECUTE_BEFORE","Framework query database before triggering this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_EXECUTE_END","Framework trigger this Hooks after query database",0);
	add_assoc_string(allHookList,"HOOKS_EXECUTE_ERROR","Framework catched CDbException will trigger this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_ERROR_HAPPEN","Framework catched Fatal error will trigger this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_EXCEPTION_HAPPEN","Framework catched Exception will trigger this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_SYSTEM_SHUTDOWN","Framework trigger this Hooks before system destory the request",0);
	add_assoc_string(allHookList,"HOOKS_CACHE_SET","Framework trigger this Hooks before write cache",0);
	add_assoc_string(allHookList,"HOOKS_CACHE_GET","Framework trigger this Hooks before read cache",0);
	add_assoc_string(allHookList,"HOOKS_LOADER_START","Framework trigger this Hooks before system init ClassLoader",0);
	add_assoc_string(allHookList,"HOOKS_VIEW_GET","Framework trigger this Hooks before system init ViewTemplate",0);
	add_assoc_string(allHookList,"HOOKS_VIEW_SHOW","Framework trigger this Hooks before display a webPage",0);
	add_assoc_string(allHookList,"HOOKS_URL_CREATE","Framework trigger this Hooks before system create a url",0);


	zend_update_static_property(CHooksCe,ZEND_STRL("_allHooks"),allHookList TSRMLS_CC);
	RETURN_ZVAL(allHookList,1,0);
}

PHP_METHOD(CHooks,_setHooksFunctionLevel)
{
	
}


//���ٱ���
void CHooks_destruct(TSRMLS_D){

	

}


void CHooks_registerHooks(char *hooksName,char *runFunctionName,zval *runObject,int callLevel TSRMLS_DC){


	char				*callClassName;
	zval				*hooksList,
						**hooksKeyList,
						**classNameKeyList,
						**runFunctionKeyList;

	zend_class_entry	*callObjectCe;

	//��ȡ����
	callObjectCe = Z_OBJCE_P(runObject);
	callClassName = estrdup(callObjectCe->name);

	//���ù���
	hooksList = zend_read_static_property(CHooksCe,ZEND_STRL("_hooks"), 0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(hooksList)){
		array_init(hooksList);
		zend_update_static_property(CHooksCe,ZEND_STRL("_hooks"),hooksList TSRMLS_CC);
	}

	//�ж�����Hooks_name��hooksName-className-runFunction��key
	if(!zend_hash_exists(Z_ARRVAL_P(hooksList),hooksName,strlen(hooksName)+1)){
		//���һ��HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(hooksList,hooksName,strlen(hooksName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_P(hooksList),hooksName,strlen(hooksName)+1,(void**)&hooksKeyList);
	

	//�ж�����className��key
	if(!zend_hash_exists(Z_ARRVAL_PP(hooksKeyList),callClassName,strlen(callClassName)+1)){
		//���һ��HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(*hooksKeyList,callClassName,strlen(callClassName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_PP(hooksKeyList),callClassName,strlen(callClassName)+1,(void**)&classNameKeyList);

	//�ж�����runFunction��key
	if(!zend_hash_exists(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1)){
		//���һ��HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(*classNameKeyList,runFunctionName,strlen(runFunctionName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1,(void**)&runFunctionKeyList);



	//���Զ�runFunctionKeyList����hooks��¼
	MODULE_BEGIN
		zval	*thisAddVal,
				*callFunctionKeyData,
				**callStatus;
		

		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1,(void**)&callStatus)){
			
			//���ô���
			add_assoc_long(*callStatus,"callNum",0);

			//���ö���
			MAKE_STD_ZVAL(thisAddVal);
			ZVAL_ZVAL(thisAddVal,runObject,1,0);
			add_assoc_zval_ex(*callStatus,"callObject",strlen("callObject")+1,thisAddVal);

			//���õȼ�
			add_assoc_long(*callStatus,"callLevel",callLevel);
	
		}

	MODULE_END

	//�������ݱ�����_hooks��
	MODULE_BEGIN
		zval *saveHooks;
		MAKE_STD_ZVAL(saveHooks);
		ZVAL_ZVAL(saveHooks,hooksList,1,0);
		zend_update_static_property(CHooksCe,ZEND_STRL("_hooks"),saveHooks TSRMLS_CC);
		zval_ptr_dtor(&saveHooks);
	MODULE_END

	efree(callClassName);

}


//ע��Hooks
PHP_METHOD(CHooks,registerHook)
{

	char	*hooksName,
			*runFunctionName,
			*callClassName;

	int		hooksNameLen,
			runFunctionNameLen;
	long	callLevel = 0;

	zval	*runObject,
			*hooksList,
			**hooksKeyList,
			**classNameKeyList,
			**runFunctionKeyList;

	zend_class_entry	*callObjectCe;


	//����4������
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ssz|l",&hooksName,&hooksNameLen,&runFunctionName,&runFunctionNameLen,&runObject,&callLevel) == FAILURE){
		RETVAL_FALSE;
		return;
	}


	//���runObject�Ƿ���һ������ �������׳��쳣
	if(IS_OBJECT != Z_TYPE_P(runObject)){
		zend_throw_exception(CPluginExceptionCe, "[HooksException] Registered hook function transfer the call object error [param 3]", "" TSRMLS_CC);
		RETVAL_FALSE;
		return;
	}

	//��ȡ����
	callObjectCe = Z_OBJCE_P(runObject);
	callClassName = estrdup(callObjectCe->name);

	//���ù���
	hooksList = zend_read_static_property(CHooksCe,ZEND_STRL("_hooks"), 0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(hooksList)){
		array_init(hooksList);
		zend_update_static_property(CHooksCe,ZEND_STRL("_hooks"),hooksList TSRMLS_CC);
	}

	//�ж�����Hooks_name��hooksName-className-runFunction��key
	if(!zend_hash_exists(Z_ARRVAL_P(hooksList),hooksName,strlen(hooksName)+1)){
		//���һ��HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(hooksList,hooksName,strlen(hooksName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_P(hooksList),hooksName,strlen(hooksName)+1,(void**)&hooksKeyList);
	

	//�ж�����className��key
	if(!zend_hash_exists(Z_ARRVAL_PP(hooksKeyList),callClassName,strlen(callClassName)+1)){
		//���һ��HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(*hooksKeyList,callClassName,strlen(callClassName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_PP(hooksKeyList),callClassName,strlen(callClassName)+1,(void**)&classNameKeyList);

	//�ж�����runFunction��key
	if(!zend_hash_exists(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1)){
		//���һ��HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(*classNameKeyList,runFunctionName,strlen(runFunctionName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1,(void**)&runFunctionKeyList);



	//���Զ�runFunctionKeyList����hooks��¼
	MODULE_BEGIN
		zval	*thisAddVal,
				*callFunctionKeyData,
				**callStatus;
		

		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1,(void**)&callStatus)){
			
			//���ô���
			add_assoc_long(*callStatus,"callNum",0);

			//���ö���
			MAKE_STD_ZVAL(thisAddVal);
			ZVAL_ZVAL(thisAddVal,runObject,1,0);
			add_assoc_zval_ex(*callStatus,"callObject",strlen("callObject")+1,thisAddVal);

			//���õȼ�
			add_assoc_long(*callStatus,"callLevel",callLevel);
	
		}

	MODULE_END

	//�������ݱ�����_hooks��
	MODULE_BEGIN
		zval *saveHooks;
		MAKE_STD_ZVAL(saveHooks);
		ZVAL_ZVAL(saveHooks,hooksList,1,0);
		zend_update_static_property(CHooksCe,ZEND_STRL("_hooks"),saveHooks TSRMLS_CC);
		zval_ptr_dtor(&saveHooks);
	MODULE_END

	efree(callClassName);

	RETVAL_TRUE;
}

PHP_METHOD(CDataObject,asArray)
{
	zval	*thisVal;
	thisVal = zend_read_property(CDataObjectCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	RETVAL_ZVAL(thisVal,1,0);
}

CHooks_setDataObject(zval *object,zval *data TSRMLS_DC){
	zend_update_property(CDataObjectCe,object,ZEND_STRL("data"),data TSRMLS_CC);
}

CHooks_getDataObject(zval *object,zval **returnData TSRMLS_DC){
	zval	*thisVal;
	thisVal = zend_read_property(CDataObjectCe,object,ZEND_STRL("data"),0 TSRMLS_CC);
	MAKE_STD_ZVAL(*returnData);
	ZVAL_ZVAL(*returnData,thisVal,1,0);
}

PHP_METHOD(CDataObject,set)
{
	zval	*data;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&data) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	//setter
	CHooks_setDataObject(getThis(),data TSRMLS_CC);
	RETVAL_TRUE;
}

PHP_METHOD(CDataObject,get)
{
	zval	*thisVal;
	thisVal = zend_read_property(CDataObjectCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	RETVAL_ZVAL(thisVal,1,0);
}