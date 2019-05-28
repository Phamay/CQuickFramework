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

//zend�����
zend_class_entry	*CActiveRecordCe;


//�෽��:����Ӧ��
PHP_METHOD(CActiveRecord,findAll);
PHP_METHOD(CActiveRecord,findByAttributes);
PHP_METHOD(CActiveRecord,find);
PHP_METHOD(CActiveRecord,asArray);
PHP_METHOD(CActiveRecord,getKey);
PHP_METHOD(CActiveRecord,current);
PHP_METHOD(CActiveRecord,save);
PHP_METHOD(CActiveRecord,add);
PHP_METHOD(CActiveRecord,count);
PHP_METHOD(CActiveRecord,update);
PHP_METHOD(CActiveRecord,delete);
PHP_METHOD(CActiveRecord,getLastError);
PHP_METHOD(CActiveRecord,tableName);


