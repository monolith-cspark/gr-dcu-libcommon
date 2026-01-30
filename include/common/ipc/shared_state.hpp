#pragma once

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <atomic>
#include <string>
#include <cstring>
#include <iostream>

namespace GR {
namespace LIBCOMMON {

namespace IPC {

/**
 * @brief 프로세스 간 공유 메모리 기반 데이터 관리 템플릿 클래스
 *
 * @tparam T 공유할 데이터 구조체 (예: SystemInitTimeline)
 * * @note
 * 1. T 내부 멤버들은 스레드/프로세스 간 경합 방지를 위해 std::atomic 사용을 권장합니다.
 * 2. 생성자(Owner)는 create()를, 사용자(User)는 open()을 호출하여 연결합니다.
 * 3. operator-> 를 통해 구조체 내부 멤버에 직접 접근하여 store/load를 수행합니다.
 *
 * 사용 예시:
 * ```cpp
 * SharedState<MyData> shm;
 * shm.open("/my_shm");
 * shm->some_atomic_value.store(10, std::memory_order_release);
 * ```
 */
template<typename T>
class SharedState {
public:
    /**
     * @brief 공유 메모리 구조체
     * atomic을 사용하여 lock-free로 상태 업데이트 해야함
     * 경합이 필요하지 않은 단순 데이터는 atomic을 사용하지 않아도 무방
     */
    // struct SharedData {
    //     std::atomic<T> state;

    //     SharedData() : state(T{}) {}
    // };

    SharedState() : shm_fd_(-1), data_ptr_(nullptr), is_owner_(false), shm_name_("") {}

    ~SharedState() {
        cleanup();
    }

    // 복사 방지
    SharedState(const SharedState&) = delete;
    SharedState& operator=(const SharedState&) = delete;

    /**
     * @brief 공유 메모리 생성 및 초기화 (Owner/Creator 용)
     */
    bool create(const std::string& shm_name) {

        if(validateName(shm_name) == false) {
            return false;
        }
        shm_name_ = shm_name;

        try {

            // 기존 공유 메모리 삭제 (있으면)
            shm_unlink(shm_name_.c_str());

            // 공유 메모리 생성 (서버가 생성 및 삭제 권한 가짐)
            shm_fd_ = shm_open(shm_name_.c_str(), O_CREAT | O_RDWR, 0666);
            if (shm_fd_ < 0) {
                std::cerr << "[SharedState] : shm_open failed: " << std::strerror(errno) << std::endl;
                return false;
            }

            // 크기 설정
            if (ftruncate(shm_fd_, sizeof(T)) < 0) {
                std::cerr << "[SharedState] : ftruncate failed: " << std::strerror(errno) << std::endl;
                ::close(shm_fd_);
                shm_fd_ = -1;
                return false;
            }

            // 메모리 맵핑
            if( mapMemory() == false) {
                return false;
            }

            if (data_ptr_) {
                new (data_ptr_) T(); // Placement new: 공유 메모리 위치에 객체 생성(초기화)
            }

            is_owner_ = true;  // 생성자 (정리 책임)

            std::cout << "[SharedState] : Created: " << shm_name_ << std::endl;
            return true;

        } catch (const std::exception& e) {
            std::cerr << "[SharedState] : Create exception: " << e.what() << std::endl;
            cleanup();
            return false;
        }
    }

    /**
     * @brief 기존 공유 메모리 연결 (User/Accessor 용)
     * 개별 에이전트에서 호출
     */
    bool open(const std::string& shm_name) {

        if(validateName(shm_name) == false) {
            return false;
        }

        shm_name_ = shm_name;

        try {
            // 기존 공유 메모리 열기 (생성 안함)
            shm_fd_ = shm_open(shm_name_.c_str(), O_RDWR, 0666);
            if (shm_fd_ < 0) {
                std::cerr << "[SharedState] : shm_open failed: " << std::strerror(errno) << std::endl;
                return false;
            }

            // 메모리 맵핑
            if( mapMemory() == false) {
                return false;
            }

            is_owner_ = false;  // 생성자 아님 (정리 안함)

            std::cout << "[SharedState] : opened: " << shm_name_ << std::endl;
            return true;

        } catch (const std::exception& e) {
            std::cerr << "[SharedState] : open exception: " << e.what() << std::endl;
            cleanup();
            return false;
        }
    }


    // 비-const 버전 (쓰기 가능)
    T* operator->() { return data_ptr_; }
    T* data() { return data_ptr_; }

    // const 버전 (읽기 전용)
    const T* operator->() const { return data_ptr_; }
    const T* data() const { return data_ptr_; }

    // 역참조 연산자
    T& operator*() { return *data_ptr_; }
    const T& operator*() const { return *data_ptr_; }

    /**
     * @brief 공유 메모리 리소스 해제
     */
    void close() {
        cleanup();
    }

    /**
     * @brief 초기화 여부 확인
     * @return true: 초기화됨, false: 초기화 안됨
     */
    bool isInitialized() const {
        return data_ptr_ != nullptr;
    }

private:
    void cleanup() {
        if (data_ptr_) {
            munmap(data_ptr_, sizeof(T));
            data_ptr_ = nullptr;
        }

        if (shm_fd_ >= 0) {
            ::close(shm_fd_);
            shm_fd_ = -1;
        }

        // Server(creator)만 shm_unlink 수행
        if (is_owner_ && !shm_name_.empty()) {
            if (shm_unlink(shm_name_.c_str()) == 0) {
                std::cout << "[SharedState] : Memory unlinked: " << shm_name_ << std::endl;
            } else {
                std::cerr << "[SharedState] : shm_unlink failed for " << shm_name_ << ": "
                          << std::strerror(errno) << std::endl;
            }
        }
    }

    bool handleInternalError(const std::string& msg) {
        std::cerr << "[SharedState] " << msg << " failed: " << std::strerror(errno) << "\n";
        cleanup();
        return false;
    }

    bool mapMemory() {
        void* ptr = mmap(NULL, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
        if (ptr == MAP_FAILED) return handleInternalError("mmap");
        data_ptr_ = static_cast<T*>(ptr);
        return true;
    }

    bool validateName(const std::string& name) {
        if (name.empty() || name[0] != '/') {
            std::cerr << "[SharedState] Invalid name: " << name << " (Must start with /)\n";
            return false;
        }
        return true;
    }

    int shm_fd_;
    T* data_ptr_;
    bool is_owner_;  // 공유 메모리 생성자 여부 (서버만 true)
    std::string shm_name_;  // 공유 메모리 이름
};

} // namespace IPC
} // namespace LIBCOMMON
} // namespace GR
