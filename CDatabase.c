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
#include "php_CDatabase.h"
#include "php_CBuilder.h"
#include "php_CException.h"


//zend�෽��
zend_function_entry CDatabase_functions[] = {
	PHP_ME(CDatabase,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDatabase,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CDatabase,getDatabase,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//ģ�鱻����ʱ
CMYFRAME_REGISTER_CLASS_RUN(CDatabase)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CDatabase",CDatabase_functions);
	CDatabaseCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//����
	zend_declare_property_null(CDatabaseCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CDatabaseCe, ZEND_STRL("objectPdo"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CDatabaseCe, ZEND_STRL("dataObject"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CDatabaseCe, ZEND_STRL("cBuilder"),ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(CDatabaseCe, ZEND_STRL("configData"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	
	return SUCCESS;
}


//ɾ�����ݿ�ʵ������
int CDatabase_deleteDatabase(int useMaster,char *dbConf TSRMLS_DC){

	zval	*instanceZval,
			*pdoListZval;
	char	thisConKey[1024];
	int		existObject = 0;

	//��̬����
	instanceZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//���õļ���
	sprintf(thisConKey,"%d%s",useMaster,dbConf);

	//�Ƿ����
	existObject = zend_hash_exists(Z_ARRVAL_P(instanceZval),thisConKey,strlen(thisConKey)+1);

	//�����ڷ����Ƴ�ʧ��
	if(!existObject){
		return 0;
	}

	//�Ƴ���������
	zend_hash_del(Z_ARRVAL_P(instanceZval),thisConKey,strlen(thisConKey)+1);

	//ɾ��objectPdo��̬����
	pdoListZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("objectPdo"),0 TSRMLS_CC);

	if(IS_ARRAY == Z_TYPE_P(pdoListZval)){
		zval *updateArray;

		if(!zend_hash_exists(Z_ARRVAL_P(pdoListZval),thisConKey,strlen(thisConKey)+1)){
			return 0;
		}

		//ִ��ɾ��
		zend_hash_del(Z_ARRVAL_P(pdoListZval),thisConKey,strlen(thisConKey)+1);
		if(zend_hash_num_elements(Z_ARRVAL_P(pdoListZval)) > 0){
			zend_update_static_property(CDatabaseCe,ZEND_STRL("objectPdo"),pdoListZval TSRMLS_CC);
		}else{
			zend_update_static_property_null(CDatabaseCe,ZEND_STRL("objectPdo") TSRMLS_CC);
		}
	}

	return 1;
}


//��ȡ��������
void CDatabase_getInstance(int useMaster,char *dbConf,zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval,
			*instanceActive,
			**thisKeyZval,
			*thisKeyZvalSave,
			*cBuilderZval;

	char	thisConKey[128];

	int		existObject;

	MAKE_STD_ZVAL(*returnZval);


	//��ȡ�������󱣴��
	instanceZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("instance"),0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(instanceZval)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CDatabaseCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		instanceZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("instance"),0 TSRMLS_CC);
	}

	//����Key
	sprintf(thisConKey,"%d%s",useMaster,dbConf);

	//�жϴ��ڸ�key������
	existObject = zend_hash_exists(Z_ARRVAL_P(instanceZval),thisConKey,strlen(thisConKey)+1);


	//���������Ի�ȡ�µ�ʵ��
	if(0 == existObject){

		zend_class_entry	**classCePP,
							*classCe;

		zval			*object,
						*saveObject;

		//��ѯ���������
		zend_hash_find(EG(class_table),"cdatabase",strlen("cdatabase")+1,(void**)&classCePP);
		classCe = *classCePP;
		//ʵ�����ò��
		MAKE_STD_ZVAL(object);
		object_init_ex(object,classCe);


		//ִ�й�����
		if (classCe->constructor) {
			zval constructReturn;
			zval constructVal;
			zval *params[2],
				 param1,
				 param2;
			params[0] = &param1;
			params[1] = &param2;
			MAKE_STD_ZVAL(params[0]);
			ZVAL_LONG(params[0],useMaster);

			MAKE_STD_ZVAL(params[1]);
			ZVAL_STRING(params[1],dbConf,1);

			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, classCe->constructor->common.function_name, 0);

			call_user_function(NULL, &object, &constructVal, &constructReturn, 2, params TSRMLS_CC);
			zval_ptr_dtor(&params[0]);
			zval_ptr_dtor(&params[1]);
			zval_dtor(&constructReturn);
		}

		//�������
		MAKE_STD_ZVAL(saveObject);
		ZVAL_ZVAL(saveObject,object,1,0);
		zend_hash_add(Z_ARRVAL_P(instanceZval),thisConKey,strlen(thisConKey)+1,&saveObject,sizeof(zval*),NULL);
		zend_update_static_property(CDatabaseCe,ZEND_STRL("instance"),instanceZval TSRMLS_CC);
		zval_ptr_dtor(&object);
	}

	//ȡ��ǰ��̬�����еĶ���
	if(zend_hash_find(Z_ARRVAL_P(instanceZval),thisConKey,strlen(thisConKey)+1,(void**)&thisKeyZval) != SUCCESS || IS_OBJECT != Z_TYPE_PP(thisKeyZval) ){
		zend_throw_exception(CDbExceptionCe, "[CMyFrameFatal]A fatal error CMyFrame extension to obtain CDatabase data object", 1007 TSRMLS_CC);
		return;
	}

	//ȡ��CBuilder����
	MAKE_STD_ZVAL(thisKeyZvalSave);
	ZVAL_ZVAL(thisKeyZvalSave,*thisKeyZval,1,0);


	cBuilderZval = zend_read_property(CDatabaseCe,thisKeyZvalSave,ZEND_STRL("cBuilder"),0 TSRMLS_CC);
	zval_ptr_dtor(&thisKeyZvalSave);

	if(IS_OBJECT == Z_TYPE_P(cBuilderZval)){
		ZVAL_ZVAL(*returnZval,cBuilderZval,1,0);
		return;
	}

	zend_throw_exception(CDbExceptionCe, "[CMyFrameFatal]A fatal error CMyFrame extension to obtain CDatabase data object", 1007 TSRMLS_CC);
}

//�෽��:����Ӧ�ö���
PHP_METHOD(CDatabase,getInstance)
{
	char	*dbConf = "main";
	int		useMaster = 0,
			dbConfLen = 0;
	zval	*returnZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|bs",&useMaster,&dbConf,&dbConfLen) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CDatabase] Call [CDatabase->getInstance] Parameter error ", 1007 TSRMLS_CC);
		return;
	}

	if(dbConfLen == 0){
		dbConf = "main";
	}


	CDatabase_getInstance(useMaster,dbConf,&returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,0);
	zval_ptr_dtor(&returnZval);
}

//���캯��
PHP_METHOD(CDatabase,__construct)
{
	char	*dbConf,
			useMaterStr[10240],
			tablePreConfKey[1024],
			*tablePre;
	long		useMaster = 1;
	int	dbConfLen = 0;
	zval	*returnZval,
			*object,
			*useMasterZval,
			*cconfigInstanceZval,
			*tablePreZval;

	zend_class_entry	**classCePP,
						*classCe;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ls",&useMaster,&dbConf,&dbConfLen) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CDatabase] Call [CDatabase->getInstance] Parameter error ", 1007 TSRMLS_CC);
		return;
	}

	//���Ի�ȡCBuilder����
	zend_hash_find(EG(class_table),"cbuilder",strlen("cbuilder")+1,(void**)&classCePP);
	classCe = *classCePP;


	//��ȡ����
	MAKE_STD_ZVAL(object);
	object_init_ex(object,classCe);

	//ִ�й�����
	if (classCe->constructor) {
		zval constructReturn;
		zval constructVal;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, classCe->constructor->common.function_name, 1);
		call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);
	}

	//���ø�CBuilder�����е�����
	sprintf(useMaterStr,"%d",useMaster);
	zend_update_property_string(CBuilderCe,object,"isMaster",strlen("isMaster"), useMaterStr TSRMLS_CC);
	zend_update_property_string(CBuilderCe,object,"configName",strlen("configName"), dbConf TSRMLS_CC);

	//д���ǰ׺
	if(strcmp(useMaterStr,"1") == 0){
		sprintf(tablePreConfKey,"%s%s%s","DB.",dbConf,".master.tablePrefix");
	}else{
		sprintf(tablePreConfKey,"%s%s%s","DB.",dbConf,".slaves.tablePrefix");
	}

	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load(tablePreConfKey,cconfigInstanceZval,&tablePreZval TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(tablePreZval)){
		tablePre = estrdup(Z_STRVAL_P(tablePreZval));
	}else{
		tablePre = estrdup("");
	}

	zend_update_property_string(CBuilderCe,object,"tablePre",strlen("tablePre"), tablePre TSRMLS_CC);
	efree(tablePre);
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&tablePreZval);

	//���������CBuilder����
	zend_update_property(CDatabaseCe,getThis(),ZEND_STRL("cBuilder"),object TSRMLS_CC);
	ZVAL_ZVAL(return_value,object,1,1);
}

//��ȡPDO���ݶ���
void CDatabase_getDatabase(char *configName,int useMaster,zval **returnZval TSRMLS_DC)
{
	zval	*pdoListZval,
			*configDataZval,
			*pdoObject;
	char	pdoKey[10240];
	int		hasExist;

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_NULL(*returnZval);

	//��ȡpdo�б�
	pdoListZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("objectPdo"),0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(pdoListZval)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CDatabaseCe,ZEND_STRL("objectPdo"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		pdoListZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("objectPdo"),0 TSRMLS_CC);
	}

	//PDO�����key
	sprintf(pdoKey,"%d%s",useMaster,configName);

	//�ж��Ƿ����PDO����
	hasExist = zend_hash_exists(Z_ARRVAL_P(pdoListZval),pdoKey,strlen(pdoKey)+1);
	if(hasExist == 0){
		
		//��ȡ�����ļ�
		char	*dbName;
		zval	*cconfigInstanceZval,
				*dbConfigZval,
				**masterZval,
				**slaveZval,
				*thisConfZval;

		//���õ���
		CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

		//��������
		strcat2(&dbName,"DB.",configName,NULL);

		//���ݿ�����
		CConfig_load(dbName,cconfigInstanceZval,&dbConfigZval TSRMLS_CC);
		efree(dbName);

		if(IS_ARRAY != Z_TYPE_P(dbConfigZval)){
			zval_ptr_dtor(&cconfigInstanceZval);
			zval_ptr_dtor(&dbConfigZval);
			zend_throw_exception(CDbExceptionCe, "[MainConfigError] CDatabase::getDataBase() method try for there is no configuration items:[Config->DB]", 1007 TSRMLS_CC);
			return;
		}
		
		//���û��master��key
		if(!zend_hash_exists(Z_ARRVAL_P(dbConfigZval),"master",strlen("master")+1)){
			zval_ptr_dtor(&cconfigInstanceZval);
			zval_ptr_dtor(&dbConfigZval);
			zend_throw_exception(CDbExceptionCe, "[MainConfigError] CDatabase::getDataBase() method try for there is no configuration items:[Config->DB->master]", 1007 TSRMLS_CC);
			return;
		}

		//master������Ϣ
		zend_hash_find(Z_ARRVAL_P(dbConfigZval),"master",strlen("master")+1,(void**)&masterZval);

		//slave������Ϣ
		if(!zend_hash_exists(Z_ARRVAL_P(dbConfigZval),"slaves",strlen("slaves")+1)){
			zend_hash_find(Z_ARRVAL_P(dbConfigZval),"master",strlen("master")+1,(void**)&slaveZval);
		}else{
			zend_hash_find(Z_ARRVAL_P(dbConfigZval),"slaves",strlen("slaves")+1,(void**)&slaveZval);
		}

		//��ȡconfigData��̬����
		configDataZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("configData"),0 TSRMLS_CC);
		if(IS_NULL == Z_TYPE_P(configDataZval)){
			array_init(configDataZval);
			zend_update_static_property(CDatabaseCe,ZEND_STRL("configData"), configDataZval TSRMLS_CC);

		}

		//�˴�������Ϣ
		MAKE_STD_ZVAL(thisConfZval);
		if(useMaster == 1){
			zval *otherConfig,
				 *saveThis;
			char otherKey[1024];
			MAKE_STD_ZVAL(otherConfig);
			ZVAL_ZVAL(otherConfig,*slaveZval,1,0);
			ZVAL_ZVAL(thisConfZval,*masterZval,1,0);

			//���浱ǰ����
			zend_hash_update(Z_ARRVAL_P(configDataZval),pdoKey, strlen(pdoKey)+1, &thisConfZval, sizeof(zval*),NULL);

			//������������
			sprintf(otherKey,"%d%s",0,configName);
			zend_hash_update(Z_ARRVAL_P(configDataZval),otherKey, strlen(otherKey)+1, &otherConfig, sizeof(zval*),NULL);
		}else{
			zval *otherConfig,
				 *saveThis;
			char otherKey[1024];

			MAKE_STD_ZVAL(otherConfig);
			ZVAL_ZVAL(otherConfig,*masterZval,1,0);
			ZVAL_ZVAL(thisConfZval,*slaveZval,1,0);

			//���浱ǰ����
			zend_hash_update(Z_ARRVAL_P(configDataZval),pdoKey, strlen(pdoKey)+1, &thisConfZval, sizeof(zval*),NULL);

			//������������
			sprintf(otherKey,"%d%s",1,configName);
			zend_hash_update(Z_ARRVAL_P(configDataZval),otherKey, strlen(otherKey)+1, &otherConfig, sizeof(zval*),NULL);
		}


		//���Ի�ȡPDO����
		MODULE_BEGIN
			zend_class_entry	**pdoPP,
								*pdoP;

			zval	**hostZval,
					**userZval,
					**passZval,
					**masterRead,
					**slavesWrite;

			char	*pdoHost,
					*user,
					*pass;

			//�Ҳ���PDO����
			if(zend_hash_find(EG(class_table),"pdo",strlen("pdo")+1,(void**)&pdoPP) == FAILURE){
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_ptr_dtor(&dbConfigZval);
				zend_throw_exception(CDbExceptionCe, "[MainConfigError] CMyFrame couldn't find the PDO object, please make sure the PDO extension is installed", 1001 TSRMLS_CC);
				return;
			}

			//��ȡ�������
			zend_hash_find(Z_ARRVAL_P(thisConfZval),"connectionString",strlen("connectionString")+1,(void**)&hostZval);
			zend_hash_find(Z_ARRVAL_P(thisConfZval),"username",strlen("username")+1,(void**)&userZval);
			zend_hash_find(Z_ARRVAL_P(thisConfZval),"password",strlen("password")+1,(void**)&passZval);

			//�жϸ������ԺϷ�
			if(IS_STRING == Z_TYPE_PP(hostZval) && IS_STRING == Z_TYPE_PP(userZval) && IS_STRING == Z_TYPE_PP(passZval) ){
			}else{
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_ptr_dtor(&dbConfigZval);
				zend_throw_exception(CDbExceptionCe, "[MainConfigError]The database configuration [DB] some key type errors", 1001 TSRMLS_CC);
				return;
			}

			if(strlen(Z_STRVAL_PP(hostZval)) <= 0 || strlen(Z_STRVAL_PP(userZval)) <= 0 ){
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_ptr_dtor(&dbConfigZval);
				zend_throw_exception(CDbExceptionCe, "[MainConfigError]Main key database configuration [DB->connectionString] or [DB->username] key cannot be empty", 1001 TSRMLS_CC);
				return;
			}

			//��ֵ
			pdoHost = estrdup(Z_STRVAL_PP(hostZval));
			user = estrdup(Z_STRVAL_PP(userZval));
			pass = estrdup(Z_STRVAL_PP(passZval));

			//ʵ������Zval�ṹ��
			pdoP = *pdoPP;
			MAKE_STD_ZVAL(pdoObject);
			object_init_ex(pdoObject,pdoP);

			//ִ���乹���� ���������
			if (pdoP->constructor) {
				zval constructReturn;
				zval constructVal,
					 *displayParam[4],
					 *queryParam[1],
					 param1,
					 param2,
					 param3,
					 param4,
					 param5;

				displayParam[0] = &param1;
				displayParam[1] = &param2;
				displayParam[2] = &param3;

				queryParam[0] = &param5;
				
				//�������
				MAKE_STD_ZVAL(displayParam[0]);
				MAKE_STD_ZVAL(displayParam[1]);
				MAKE_STD_ZVAL(displayParam[2]);
				ZVAL_STRING(displayParam[0],pdoHost,1);
				ZVAL_STRING(displayParam[1],user,1);
				ZVAL_STRING(displayParam[2],pass,1);

				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, pdoP->constructor->common.function_name, 0);
				call_user_function(NULL, &pdoObject, &constructVal, &constructReturn, 3,displayParam TSRMLS_CC);

				zval_ptr_dtor(&displayParam[0]);
				zval_ptr_dtor(&displayParam[1]);
				zval_ptr_dtor(&displayParam[2]);
				zval_dtor(&constructReturn);


				//����쳣 ��ΪPDO��תΪCDbException�쳣�׳�
				MODULE_BEGIN
					if(EG(exception)){

						//ȷ���쳣���Ƿ�ΪPDOException
						zend_class_entry *exceptionCe;

						exceptionCe = Z_OBJCE_P(EG(exception));
						if(strcmp(exceptionCe->name,"PDOException") == 0){
							
							//��ȡ�������Ϣ
							zval *exceptionMessage;
							char *pdoErrStr;
							exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);
							strcat2(&pdoErrStr,"[CDbException] ",Z_STRVAL_P(exceptionMessage),", Unable to connect to the database using the specified configuration",NULL);
							zend_clear_exception(TSRMLS_C);

							zval_ptr_dtor(&cconfigInstanceZval);
							zval_ptr_dtor(&dbConfigZval);
							zval_ptr_dtor(&pdoObject);
							efree(user);
							efree(pdoHost);
							efree(pass);

							zend_throw_exception(CDbExceptionCe, pdoErrStr, 1001 TSRMLS_CC);
							return;
						}
					}
				MODULE_END

				//�����ַ���
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, "query", 0);
				MAKE_STD_ZVAL(queryParam[0]);
				ZVAL_STRING(queryParam[0],"set names utf8",1);
				call_user_function(NULL, &pdoObject, &constructVal, &constructReturn, 1,queryParam TSRMLS_CC);
				zval_ptr_dtor(&queryParam[0]);
				

				//��PDO�����ھ�̬������
				zend_hash_update(Z_ARRVAL_P(pdoListZval),pdoKey, strlen(pdoKey)+1, &pdoObject, sizeof(zval*),NULL);

				//����һ������
				ZVAL_ZVAL(*returnZval,pdoObject,1,0);
				zval_ptr_dtor(&dbConfigZval);
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_dtor(&constructReturn);
				efree(user);
				efree(pdoHost);
				efree(pass);
				return;
			}


		MODULE_END

		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&dbConfigZval);

		//���󱨸�
		zend_throw_exception(CDbExceptionCe, "[DatabaseConnectionError] Unable to connect to the database using the specified configuration", 1007 TSRMLS_CC);
		return;
	}else{

		//ֱ�ӷ���PDO����
		zval	**pdoObject;
		zend_hash_find(Z_ARRVAL_P(pdoListZval),pdoKey,strlen(pdoKey)+1,(void**)&pdoObject);
		ZVAL_ZVAL(*returnZval,*pdoObject,1,0);
		return;
	}

}

//��ȡPDO���ݶ���
PHP_METHOD(CDatabase,getDatabase)
{
	zval	*returnZval,
			*userMasterZval;
	char	*configName,
			*defaultConfigName = "main";
	int		configNameLen = 0,
			userMaster = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|sz",&configName,&configNameLen,&userMasterZval) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CDatabase] Call [CDatabase->getDatabase] Parameter error ", 1007 TSRMLS_CC);
		RETVAL_FALSE;
		return;
	}

	if(userMasterZval != NULL && IS_BOOL == Z_TYPE_P(userMasterZval) && Z_LVAL_P(userMasterZval) == 1 ){
		userMaster = 1;
	}

	CDatabase_getDatabase(defaultConfigName,userMaster,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}