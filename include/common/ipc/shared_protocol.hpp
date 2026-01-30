#pragma once

#include <atomic>
#include <cstdint>

namespace GR {
namespace LIBCOMMON {
namespace IPC {
    
/**
 * 레이아웃 정렬을 위해 8바이트 단위로 설계
 */


// 공유 메모리 이름 정의
inline constexpr const char* SOUND_SHM_NAME = "/sound_ipc_shm";

// Heartbeat 임계값 (ms)
inline constexpr const int64_t AliveTimeThresholdSound = 5000;  // 5

// 네트워크 지연, 스케줄링 등을 고려한 추가 여유 시간 (Margin)
inline constexpr const int64_t AliveTimeMargin = 500;



// ============================================================
// 시스템 초기화 타임라인 추적 구조체
// ============================================================

inline constexpr const char* SYSTEM_INIT_TIMELINE_SHM_NAME = "/gr_system_init_timeline";

enum class InitMetricState : uint8_t {
    NOT_STARTED = 0,
    IN_PROGRESS = 1,
    DONE        = 2,
    FAILED      = 3,
    REPORTED    = 4
};

struct InitTimeMetric {
    std::atomic<InitMetricState> state;
    uint8_t  reserved[7];   // 8바이트 정렬

    std::atomic<uint64_t> end_time_ms;
    std::atomic<uint64_t> duration_ms;
};

struct SystemInitTimeline {

    // ===== GPS =====
    InitTimeMetric gps_ttff;          // Time To First Fix
    InitTimeMetric gps_rtk_fix;        // RTK FIX 소요 시간

    // ===== Network =====
    InitTimeMetric network_up;         // link up
    InitTimeMetric ntrip_connected;    // NTRIP 연결
    InitTimeMetric mqtt_connected;     // MQTT 브로커 연결

    // ===== 전체 =====
    InitTimeMetric system_ready;       // 모든 준비 완료 시점

    std::atomic<uint64_t> last_update_ms;
    std::atomic<uint64_t> start_time_ms;
};



/**
 * 사운드용 데이터 구조체
 */
enum class SoundState : uint8_t {
    IDLE                = 0, // 초기 대기 상태
    STARTING_UP         = 1, // 시스템(MQ/SHM) 초기화 중
    ENGINE_INIT_READY   = 2, // 오디오 엔진 하드웨어 연결 성공
    RESOURCE_LOAD_READY = 3, // 사운드 파일 로드 완료
    RUNNING             = 4, // 모든 준비 완료 및 정상 작동
    DISABLED            = 5, // 에이전트 비활성화 상태
    
    // Error States
    HARDWARE_FAILURE    = 6, // 사운드 장치를 찾을 수 없음
    RESOURCE_MISSING    = 7, // 사운드 파일 경로 오류
    MESSAGE_BUS_ERROR   = 8, // MQ 통신 장애
    UNKNOWN_ERROR       = 9  // 기타 예외 상황
};

struct SoundIpcData {
    // 현재는 mq에서 제어하지만, 향후 확장 가능
    struct Config {
        std::atomic<uint8_t> master_volume;
        std::atomic<bool> mute_request;
        uint8_t reserved[6];
    };

    struct Status {
        std::atomic<SoundState> state;
        std::atomic<bool> is_active;
        uint8_t padding[6];
        std::atomic<uint64_t> heartbeat;
    };

    Config server_to_client;    // Domain Controller -> Sound Agent
    Status client_to_server;    // Sound Agent -> Domain Controller
};

// ============================================================
// GPS 옵션 플래그
// ============================================================
enum class GpsOption : uint8_t {
    NONE         = 0,
    USE_RTK      = 1 << 0,  // 0x01
    USE_DR       = 1 << 1,  // 0x02
    IMU_SAVE     = 1 << 2,  // 0x04
    IMU_RESTORE  = 1 << 3   // 0x08
};

// ============================================================
// Device Config (GPS, IMU)
// ============================================================

inline constexpr const char* DEVICE_CONFIG_SHM_NAME = "/gr_device_config";

/**
 * @brief 개별 디바이스 설정 구조체
 */
struct DeviceConfig {
    char     port[32];          // 시리얼 포트 경로 (예: "/dev/ttyAMA0")
    uint32_t baudrate;          // 통신 속도 (예: 9600, 115200)
    uint16_t update_rate_hz;    // 업데이트 주기 (Hz)
    uint8_t  option;            // 옵션
    uint8_t  type;              // 타입
    bool     enabled;           // 활성화 여부
    uint8_t  reserved[4];       // 향후 확장용

    DeviceConfig()
        : baudrate(115200)
        , update_rate_hz(10)
        , option(0)
        , type(1)  // 1 = SERIAL
        , enabled(false)
    {
        port[0] = '\0';
        std::memset(reserved, 0, sizeof(reserved));
    }

    void setPort(const char* p) {
        std::strncpy(port, p, sizeof(port) - 1);
        port[sizeof(port) - 1] = '\0';
    }

};

/**
 * @brief 전체 디바이스 설정 테이블
 *
 * DCU(Server)에서 설정 후 각 Agent(Client)가 읽어감
 */
struct DeviceConfigTable {
    DeviceConfig gps;
    DeviceConfig imu;

    std::atomic<bool> ready;  // 서버 설정 완료 플래그

    struct AgentStatus {
        std::atomic<bool> gps_ready;
        std::atomic<bool> imu_ready;
    } status;

    DeviceConfigTable() : ready(false) {
        status.gps_ready.store(false);
        status.imu_ready.store(false);
    }
};

namespace Utils {

    inline bool HasGpsOption(uint8_t currentOptions, GpsOption target) {
        return (currentOptions & static_cast<uint8_t>(target)) != 0;
    }
}


} // namespace IPC
} // namespace LIBCOMMON
} // namespace GR