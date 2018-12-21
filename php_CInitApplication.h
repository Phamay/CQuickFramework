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
zend_class_entry	*CInitApplicationCe;


//�෽��:����Ӧ��
PHP_METHOD(CInitApplication,__construct);
PHP_METHOD(CInitApplication,webShutdown);
PHP_METHOD(CInitApplication,GetRequest);
PHP_METHOD(CInitApplication,classAutoLoad);


PHP_METHOD(CInitApplication,_initAutoLoad);
PHP_METHOD(CInitApplication,_initTimeZone);
PHP_METHOD(CInitApplication,_initIncludeClass);
PHP_METHOD(CInitApplication,_initTopException);
PHP_METHOD(CInitApplication,_initHooks);
PHP_METHOD(CInitApplication,_initGzip);
PHP_METHOD(CInitApplication,_initCookie);
PHP_METHOD(CInitApplication,_initSession);
