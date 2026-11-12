#include <jni.h>
#include <string>
#include <vector>
#include <memory>
#include "sentencepiece_lite.h"

// نگهداری نمونه مدل در رم native
static std::unique_ptr<sentencepiece::SentencePieceLite> g_sp = nullptr;

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

    std::string_view buffer_view(reinterpret_cast<const char*>(buf), len);
    
    g_sp = std::make_unique<sentencepiece::SentencePieceLite>();
    auto status = g_sp->LoadFromBuffer(buffer_view);
    
    env->ReleaseByteArrayElements(model_buffer, buf, JNI_ABORT);
    
    return status.ok() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jintArray JNICALL
Java_com_logcat_puncfixer_SentencePieceLite_encode(
        JNIEnv* env,
        jobject /* this */,
        jstring text) {
    
    if (!g_sp || text == nullptr) return nullptr;

    const char* native_str = env->GetStringUTFChars(text, nullptr);
    std::string_view sv(native_str);

    std::vector<int> ids;
    auto status = g_sp->Encode(sv, &ids);
    
    env->ReleaseStringUTFChars(text, native_str);

    if (!status.ok()) return nullptr;

    jintArray result = env->NewIntArray(ids.size());
    if (result == nullptr) return nullptr;

    env->SetIntArrayRegion(result, 0, ids.size(), reinterpret_cast<const jint*>(ids.data()));
    return result;
}

} // extern "C"
