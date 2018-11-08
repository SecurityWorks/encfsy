#pragma once

#include <string>
#include <mutex>
#include <exception>

#include <modes.h>
#include <pwdbased.h>
#include <sha.h>
#include <osrng.h>

using namespace std;
using namespace CryptoPP;

namespace EncFS
{
	/**
	EncFS�̃{�����[���ݒ�ł��B
	���̃N���X�͊�b�I�ȈÍ����A�������@�\��񋟂��Ă��܂��B
	loadConfig(), unlock() �������ă}���`�X���b�h�ɑΉ����Ă��܂��B
	**/
	class EncFSVolume {
	public:
		/** �u���b�N�̃w�b�_�[�T�C�Y�B **/
		static const size_t HEADER_SIZE = 8;

	private:
		/** �Í����L�[�̃T�C�Y�B192�܂���256�B */
		int keySize;
		/** �t�@�C�����̃u���b�N�T�C�Y�B1024�����B */
		int blockSize;
		bool uniqueIV;
		/** �t�@�C�����̈Í�������ʂ̃f�B���N�g�����Ɉˑ������邩�B */
		bool chainedNameIV;
		/** �t�@�C�����̈Í�������ʂ̃f�B���N�g�����Ɉˑ������邩�B */
		bool externalIVChaining;
		int blockMACBytes;
		int blockMACRandBytes;
		bool allowHoles;

		int encodedKeySize;
		string encodedKeyData;

		int saltLen;
		string saltData;

		/** �����o�֐��̌J��Ԃ��񐔁B */
		int kdfIterations;
		/**  �����o�֐��ɂ����錩�ς��莞�ԁB�@*/
		int desiredKDFDuration;

		string volumeKey;
		string volumeIv;

		HMAC<SHA1> volumeHmac;
		mutex hmacLock;

		int base64Lookup[256];

		// AES / CBC / NoPadding
		CBC_Mode<AES>::Encryption aesCbcEnc;
		mutex aesCbcEncLock;
		CBC_Mode<AES>::Decryption aesCbcDec;
		mutex aesCbcDecLock;

		// AES / CFB / NoPadding
		CFB_Mode<AES>::Encryption aesCfbEnc;
		mutex aesCfbEncLock;
		CFB_Mode<AES>::Decryption aesCfbDec;
		mutex aesCfbDecLock;

	public:
		EncFSVolume();
		~EncFSVolume() {};

		/**
		EncFS�̐ݒ��ǂݍ��݂܂��B���̌`���ɌŒ肳��܂��B
		.encfs6.xml �`��
		cipherAlg ssl/aes 3.0
		nameAlg nameio/block 3.0
		**/
		void load(const string &xml);

		void create(char* password);

		void save(string &xml);

		inline size_t getHeaderSize() {
			return this->blockMACRandBytes + this->blockMACBytes;
		}
		inline size_t getBlockSize() {
			return this->blockSize;
		}
		inline bool isChainedNameIV() {
			return this->chainedNameIV;
		}
		inline bool isExternalIVChaining() {
			return this->externalIVChaining;
		}

		/**
		�{�����[���̈Í������������ēǂݏ����̑�������s�\�ɂ��܂��B
		**/
		void unlock(char* password);

		void encodeFileName(const string &plainFileName, const string &plainDirPath, string &encodedFileName);
		void decodeFileName(const string &encodedFileName, const string &plainDirPath, string &plainFileName);
		void encodeFilePath(const string &plainFilePath, string &encodedFilePath);
		size_t toDecodedLength(const size_t encodedLength);
		size_t toEncodedLength(const size_t decodedLength);

		void encodeFileIv(const string &plainFilePath, const int64_t fileIv, string &encodedFileHeader);
		int64_t decodeFileIv(const string &plainFilePath, const string &encodedFileHeader);

		void encodeBlock(const int64_t fileIv, const int64_t blockNum, const string &plainBlock, string &encodedBlock);
		void decodeBlock(const int64_t fileIv, const int64_t blockNum, const string &encodedBlock, string &plainBlock);

	private:
		void deriveKey(char* password, string &pbkdf2Key);
		void processFileName(SymmetricCipher &cipher, mutex &cipherLock, const string &fileIv, const string &binFileName, string &fileName);
		void codeBlock(const int64_t fileIv, const int64_t blockNum, const bool encode, const string &encodedBlock, string &plainBlock);
	};

	class EncFSBadConfigurationException : runtime_error {
	public:
		EncFSBadConfigurationException(const char* what) : runtime_error(what) {};
		EncFSBadConfigurationException(const string &what) : runtime_error(what) {};
		~EncFSBadConfigurationException() {};
		const char* what() const noexcept {
			return runtime_error::what();
		}
	};

	class EncFSUnlockFailedException : exception {
	public:
		EncFSUnlockFailedException() {}
		~EncFSUnlockFailedException() {};

		const char* what() const noexcept {
			return "�L�[�𕜍��ł��܂���ł����B";
		}
	};

	class EncFSInvalidBlockException : exception {
	public:
		EncFSInvalidBlockException() {}
		~EncFSInvalidBlockException() {};

		const char* what() const noexcept {
			return "�s�K�؂ȈÍ������s���Ă��܂��B";
		}
	};
}