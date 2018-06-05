/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
#include "madara/MadaraExport.h"
/* Header for class com_madara_containers_FlexMap */

#ifndef _Included_com_madara_containers_FlexMap
#define _Included_com_madara_containers_FlexMap
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_FlexMap
 * Signature: ()J
 */
MADARA_EXPORT jlong JNICALL
Java_com_madara_containers_FlexMap_jni_1FlexMap__
  (JNIEnv *, jobject);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_FlexMap
 * Signature: (J)J
 */
MADARA_EXPORT jlong JNICALL
Java_com_madara_containers_FlexMap_jni_1FlexMap__J
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_freeFlexMap
 * Signature: (J)V
 */
MADARA_EXPORT void JNICALL
Java_com_madara_containers_FlexMap_jni_1freeFlexMap
  (JNIEnv *, jclass, jlong);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_clear
 * Signature: (JZ)V
 */
MADARA_EXPORT void JNICALL
Java_com_madara_containers_FlexMap_jni_1clear
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_erase
 * Signature: (JLjava/lang/String;)V
 */
MADARA_EXPORT void JNICALL
Java_com_madara_containers_FlexMap_jni_1erase
  (JNIEnv *, jobject, jlong, jstring);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_setString
 * Signature: (JLjava/lang/String;)V
 */
MADARA_EXPORT void JNICALL
Java_com_madara_containers_FlexMap_jni_1setString
  (JNIEnv *, jobject, jlong, jstring);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_setDouble
 * Signature: (JD)V
 */
MADARA_EXPORT void JNICALL
Java_com_madara_containers_FlexMap_jni_1setDouble
  (JNIEnv *, jobject, jlong, jdouble);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_set
 * Signature: (JJJ)V
 */
MADARA_EXPORT void JNICALL
Java_com_madara_containers_FlexMap_jni_1set
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_getName
 * Signature: (J)Ljava/lang/String;
 */
MADARA_EXPORT jstring JNICALL
Java_com_madara_containers_FlexMap_jni_1getName
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_setName
 * Signature: (JJJLjava/lang/String;)V
 */
MADARA_EXPORT void JNICALL
Java_com_madara_containers_FlexMap_jni_1setName
  (JNIEnv *, jobject, jlong, jlong, jlong, jstring);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_get
 * Signature: (JLjava/lang/String;)J
 */
MADARA_EXPORT jlong JNICALL
Java_com_madara_containers_FlexMap_jni_1get
  (JNIEnv *, jobject, jlong, jstring);

/*
* Class:     com_madara_containers_FlexMap
* Method:    jni_getIndex
* Signature: (JI)J
*/
MADARA_EXPORT jlong JNICALL
Java_com_madara_containers_FlexMap_jni_1getIndex
(JNIEnv *, jobject, jlong, jint);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_toRecord
 * Signature: (J)J
 */
MADARA_EXPORT jlong JNICALL
Java_com_madara_containers_FlexMap_jni_1toRecord
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_modify
 * Signature: (J)V
 */
MADARA_EXPORT void JNICALL
Java_com_madara_containers_FlexMap_jni_1modify
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_madara_containers_FlexMap
 * Method:    jni_toMapContainer
 * Signature: (J)J
 */
MADARA_EXPORT jlong JNICALL
Java_com_madara_containers_FlexMap_jni_1toMapContainer
  (JNIEnv *, jobject, jlong);


/*
* Class:     com_madara_containers_FlexMap
* Method:    jni_getDelimiter
* Signature: (J)Ljava/lang/String;
*/
MADARA_EXPORT jstring JNICALL
Java_com_madara_containers_FlexMap_jni_1getDelimiter
(JNIEnv *, jobject, jlong);

/*
* Class:     com_madara_containers_FlexMap
* Method:    jni_setDelimiter
* Signature: (JLjava/lang/String;)V
*/
MADARA_EXPORT void JNICALL
Java_com_madara_containers_FlexMap_jni_1setDelimiter
(JNIEnv *, jobject, jlong, jstring);

/*
* Class:     com_madara_containers_FlexMap
* Method:    jni_keys
* Signature: (JZ)[Ljava/lang/String;
*/
MADARA_EXPORT jobjectArray JNICALL
Java_com_madara_containers_FlexMap_jni_1keys
(JNIEnv *, jobject, jlong, jboolean);

/*
* Class:     com_madara_containers_FlexMap
* Method:    jni_setSettings
* Signature: (JJ)V
*/
MADARA_EXPORT void JNICALL
Java_com_madara_containers_FlexMap_jni_1setSettings
(JNIEnv *, jobject, jlong, jlong);

/*
* Class:     com_madara_containers_FlexMap
* Method:    jni_isTrue
* Signature: (J)Z
*/
MADARA_EXPORT jboolean JNICALL
Java_com_madara_containers_FlexMap_jni_1isTrue
(JNIEnv *, jobject, jlong);

/*
* Class:     com_madara_containers_FlexMap
* Method:    jni_isFalse
* Signature: (J)Z
*/
MADARA_EXPORT jboolean JNICALL
Java_com_madara_containers_FlexMap_jni_1isFalse
(JNIEnv *, jobject, jlong);


#ifdef __cplusplus
}
#endif
#endif
