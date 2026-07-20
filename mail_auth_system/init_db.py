import pymysql

# 데이터베이스 접속 정보
DB_CONFIG = {
    'host': 'localhost',
    'port': 3306,
    'user': 'omity_user',
    'password': 'omity_pass123!',  # 아까 설정한 비밀번호
    'db': 'omity_db',
    'charset': 'utf8mb4',
    'cursorclass': pymysql.cursors.DictCursor
}

def create_tables():
    # 1. MySQL 데이터베이스 연결
    try:
        connection = pymysql.connect(**DB_CONFIG)
        print("MySQL 데이터베이스 연결 성공!")
    except Exception as e:
        print(f"DB 연결 실패: {e}")
        print("1단계에서 mysql-server를 실행했는지, 혹은 계정 설정을 올바르게 했는지 확인해주세요.")
        return

    try:
        with connection.cursor() as cursor:
            # Foreign Key 제약 조건을 체크하기 위해 순서대로 테이블 생성
            
            # 2. users 테이블 생성
            print("🕒 users 테이블 생성 중...")
            cursor.execute("""
                CREATE TABLE IF NOT EXISTS users (
                    user_id INT AUTO_INCREMENT PRIMARY KEY,
                    email VARCHAR(255) NOT NULL UNIQUE,
                    password_hash VARCHAR(255) NOT NULL,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
            """)

            # 3. licenses 테이블 생성 (users와 연결)
            print("🕒 licenses 테이블 생성 중...")
            cursor.execute("""
                CREATE TABLE IF NOT EXISTS licenses (
                    license_id INT AUTO_INCREMENT PRIMARY KEY,
                    user_id INT NOT NULL,
                    license_key VARCHAR(255) NOT NULL UNIQUE,
                    status VARCHAR(50) DEFAULT 'ACTIVE',
                    hardware_uuid VARCHAR(255) DEFAULT NULL,
                    expires_at DATETIME NOT NULL,
                    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
                ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
            """)

            # 4. email_accounts 테이블 생성 (users와 연결)
            print("🕒 email_accounts 테이블 생성 중...")
            cursor.execute("""
                CREATE TABLE IF NOT EXISTS email_accounts (
                    account_id INT AUTO_INCREMENT PRIMARY KEY,
                    user_id INT NOT NULL,
                    smtp_email VARCHAR(255) NOT NULL,
                    smtp_password_encrypted TEXT NOT NULL,
                    smtp_server VARCHAR(255) NOT NULL,
                    smtp_port INT NOT NULL,
                    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
                ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
            """)

            # 변경사항 최종 반영 (Commit)
            connection.commit()
            print("모든 테이블이 성공적으로 생성되었습니다!")

    except Exception as e:
        print(f"테이블 생성 중 오류 발생: {e}")
    finally:
        connection.close()
        print("DB 연결 해제 완료.")

if __name__ == "__main__":
    create_tables()
