import os
import pymysql
from cryptography.fernet import Fernet

DB_CONFIG = {
    'host': 'localhost',
    'port': 3306,
    'user': 'omity_user',
    'password': 'omity_pass123!',
    'db': 'omity_db',
    'charset': 'utf8mb4',
    'cursorclass': pymysql.cursors.DictCursor
}

KEY_FILE = ".encryption_key"

def get_or_create_key() -> bytes:
    """
    비밀번호 암호화에 사용할 고유 마스터 키를 가져오거나 생성.
    이 키 파일이 유출되지 않도록 주의.
    """
    if os.path.exists(KEY_FILE):
        with open(KEY_FILE, "rb") as f:
            return f.read()
    else:
        
        key = Fernet.generate_key()
        with open(KEY_FILE, "wb") as f:
            f.write(key)
        return key

ENCRYPTION_KEY = get_or_create_key()
cipher = Fernet(ENCRYPTION_KEY)


def encrypt_password(password: str) -> str:
    """메일 앱 비밀번호를 양방향 암호화."""
    return cipher.encrypt(password.encode('utf-8')).decode('utf-8')


def decrypt_password(encrypted_password: str) -> str:
    """암호화된 비밀번호를 원래 문자열로 복호화."""
    return cipher.decrypt(encrypted_password.encode('utf-8')).decode('utf-8')


def add_email_account(user_id: int, smtp_email: str, smtp_password: str, smtp_server: str, smtp_port: int) -> bool:
    """
    사용자의 전송용 이메일 계정 정보(SMTP)를 등록.
    비밀번호는 암호화되어 DB에 안전하게 저장.
    """
    connection = pymysql.connect(**DB_CONFIG)
    try:
    
        encrypted_pw = encrypt_password(smtp_password)

        with connection.cursor() as cursor:
            
            query = """
                INSERT INTO email_accounts (user_id, smtp_email, smtp_password_encrypted, smtp_server, smtp_port)
                VALUES (%s, %s, %s, %s, %s)
            """
            cursor.execute(query, (user_id, smtp_email, encrypted_pw, smtp_server, smtp_port))
            connection.commit()
            print(f"이메일 계정 등록 성공 ({smtp_email})")
            return True

    except Exception as e:
        print(f"이메일 계정 등록 실패: {e}")
        connection.rollback()
        return False
    finally:
        connection.close()


def get_user_email_accounts(user_id: int) -> list:
    """
    특정 사용자가 등록한 모든 전송용 이메일 계정 리스트.
    꺼내올 때 비밀번호는 알아서 원래 비밀번호로 복호화되어 반환.
    """
    connection = pymysql.connect(**DB_CONFIG)
    accounts = []
    try:
        with connection.cursor() as cursor:
            query = """
                SELECT account_id, smtp_email, smtp_password_encrypted, smtp_server, smtp_port
                FROM email_accounts
                WHERE user_id = %s
            """
            cursor.execute(query, (user_id,))
            rows = cursor.fetchall()

            for row in rows:
                
                decrypted_pw = decrypt_password(row['smtp_password_encrypted'])
                accounts.append({
                    'account_id': row['account_id'],
                    'smtp_email': row['smtp_email'],
                    'smtp_password': decrypted_pw,
                    'smtp_server': row['smtp_server'],
                    'smtp_port': row['smtp_port']
                })
        return accounts

    except Exception as e:
        print(f"이메일 계정 불러오기 실패: {e}")
        return []
    finally:
        connection.close()
