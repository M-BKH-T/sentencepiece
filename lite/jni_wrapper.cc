#include <jni.h>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include "sentencepiece_lite.h"

using sentencepiece::lite::SentencePieceLiteProcessor;
using sentencepiece::lite::StatusCode;

// نگهداری بافر مدل و نمونه پردازشگر در حافظه Native
static std::shared_ptr<std::string> g_model_buffer = nullptr;
static std::unique_ptr<SentencePieceLiteProcessor> g_sp = nullptr;

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_logcat_puncfixer_SentencePieceLite_loadModel(
        JNIEnv* env,
        jobject /* this */,
        jbyteArray model_buffer) {
    
    if (model_buffer == nullptr) return JNI_FALSE;
    
    jsize len = env->GetArrayLength(model_buffer);
    jbyte* buf = env->GetByteArrayElements(model_buffer, nullptr);
    if (!buf) return JNI_FALSE;

    // نگهداری بافر مدل در رم جهت ارجاع مستقیم
    g_model_buffer = std::make_shared<std::string>(reinterpret_cast<const char*>(buf), len);
    env->ReleaseByteArrayElements(model_buffer, buf, JNI_ABORT);

    // ساخت پردازشگر با انتقال مالکیت بافر
    g_sp = std::make_unique<SentencePieceLiteProcessor>(g_model_buffer);
    
    return (g_sp->status() == StatusCode::kOk) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jintArray JNICALL
Java_com_logcat_puncfixer_SentencePieceLite_encode(
        JNIEnv* env,
        jobject /* this */,
        jstring text) {
    
    if (!g_sp || text == nullptr) return nullptr;

    const char* native_str = env->GetStringUTFChars(text, nullptr);
    if (!native_str) return nullptr;

    std::string_view sv(native_str);
    std::vector<int> ids;
    auto status = g_sp->Encode(sv, &ids);
    
    env->ReleaseStringUTFChars(text, native_str);

    if (status != StatusCode::kOk) return nullptr;

    jintArray result = env->NewIntArray(ids.size());
    if (result == nullptr) return nullptr;

    env->SetIntArrayRegion(result, 0, ids.size(), reinterpret_cast<const jint*>(ids.data()));
    return result;
}

} // extern "C"
