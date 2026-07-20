import hashlib
import uuid
import subprocess
import platform
import pymysql
from datetime import datetime, timedelta

DB_CONFIG = {
    'host': 'localhost',
    'port': 3306,
    'user': 'omity_user',
    'password': 'omity_pass123!',
    'db': 'omity_db',
    'charset': 'utf8mb4',
    'cursorclass': pymysql.cursors.DictCursor
}

def hash_password(password: str) -> str:
    return hashlib.sha256(password.encode('utf-8')).hexdigest()

def generate_license_key() -> str:
    raw_uuid = str(uuid.uuid4()).upper().split('-')
    return f"OMITY-{raw_uuid[0]}-{raw_uuid[1]}-{raw_uuid[2]}"

def get_hardware_uuid() -> str:
    os_type = platform.system()
    try:
        if os_type == "Windows":
            cmd = "wmic csproduct get uuid"
            output = subprocess.check_output(cmd, shell=True).decode().split()
            return output[1] if len(output) > 1 else "UNKNOWN_WINDOWS_UUID"
        elif os_type == "Linux":
            try:
                with open("/sys/class/dmi/id/product_uuid", "r") as f:
                    return f.read().strip()
            except IOError:
                with open("/etc/machine-id", "r") as f:
                    return f.read().strip()
        elif os_type == "Darwin":
            cmd = "system_profiler SPHardwareDataType | awk '/Hardware UUID/ {print $3}'"
            output = subprocess.check_output(cmd, shell=True).decode().strip()
            return output
    except Exception as e:
        return f"ERROR_OBTAINING_UUID: {str(e)}"
    return "UNSUPPORTED_OS"

def register_user(email: str, password: str) -> bool:
    """
    1. 신규 사용자를 회원가입 시키고, 
    2. 동시에 가입 즉시 30일짜리 테스트용 라이선스를 자동으로 발급.
    """
    connection = pymysql.connect(**DB_CONFIG)
    try:
        with connection.cursor() as cursor:
            cursor.execute("SELECT user_id FROM users WHERE email = %s", (email,))
            if cursor.fetchone():
                print(f"가입 실패: 이미 가입된 이메일입니다. ({email})")
                return False

            hashed_pw = hash_password(password)
            cursor.execute(
                "INSERT INTO users (email, password_hash) VALUES (%s, %s)",
                (email, hashed_pw)
            )
            user_id = cursor.lastrowid

            new_key = generate_license_key()
            expire_date = datetime.now() + timedelta(days=14)
            
            cursor.execute(
                """
                INSERT INTO licenses (user_id, license_key, status, expires_at)
                VALUES (%s, %s, 'ACTIVE', %s)
                """,
                (user_id, new_key, expire_date)
            )

            connection.commit()
            print(f"회원가입 성공! (유저 ID: {user_id})")
            print(f"발급된 라이선스 키: {new_key} (만료일: {expire_date.strftime('%Y-%m-%d')})")
            return True

    except Exception as e:
        print(f"회원가입 오류: {e}")
        connection.rollback()
        return False
    finally:
        connection.close()


def verify_license(email: str, license_key: str) -> bool:
    """
    사용자가 입력한 이메일과 라이선스 키가 유효한지, 
    그리고 현재 PC의 하드웨어 UUID와 일치하는지 확인.
    """
    current_hw_uuid = get_hardware_uuid()
    connection = pymysql.connect(**DB_CONFIG)
    
    try:
        with connection.cursor() as cursor:
            query = """
                SELECT l.license_id, l.status, l.hardware_uuid, l.expires_at
                FROM licenses l
                JOIN users u ON l.user_id = u.user_id
                WHERE u.email = %s AND l.license_key = %s
            """
            cursor.execute(query, (email, license_key))
            license_info = cursor.fetchone()

            if not license_info:
                print("인증 실패: 이메일 또는 라이선스 키가 올바르지 않습니다.")
                return False

            if license_info['status'] != 'ACTIVE':
                print(f"인증 실패: 비활성화 상태인 라이선스입니다. (상태: {license_info['status']})")
                return False

            if license_info['expires_at'] < datetime.now():
                print("인증 실패: 만료된 라이선스입니다.")
                return False

            db_hw_uuid = license_info['hardware_uuid']

            if db_hw_uuid is None:
                cursor.execute(
                    "UPDATE licenses SET hardware_uuid = %s WHERE license_id = %s",
                    (current_hw_uuid, license_info['license_id'])
                )
                connection.commit()
                print("최초 실행 확인: 현재 PC 고유 ID를 라이선스에 등록했습니다!")
                return True
                
            elif db_hw_uuid != current_hw_uuid:
                print("인증 실패: 다른 컴퓨터에서 이미 사용 중인 라이선스 키입니다! (부정 사용 감지)")
                return False

            print("인증 성공! 환영합니다. 프로그램을 시작합니다.")
            return True

    except Exception as e:
        print(f"라이선스 인증 검증 오류: {e}")
        return False
    finally:
        connection.close()
