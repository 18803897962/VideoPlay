#pragma once
#include<atomic>
#include <vector>
#include<mutex>
#include <windows.h>
#include<varargs.h>
void MYTRACE(const char* format, ...) {
	va_list ap;
	va_start(ap,format);
	std::string sBuffer;
	sBuffer.resize(1024 * 10);
	vsprintf((char*)(sBuffer.c_str()), format, ap);
	OutputDebugStringA(sBuffer.c_str());
	va_end(ap);
}
#ifndef TRACE
#define TRACE MYTRACE
#endif // !TRACE

class ThreadFuncBase {//�߳̽ӿ�
};
typedef int (ThreadFuncBase::* FUNCTYPE)();//ThreadFuncBase��Ա����ָ��
class ThreadWorker {
public:
	ThreadWorker():thiz(NULL),func(NULL){}
	ThreadWorker(void* obj,FUNCTYPE functype):thiz((ThreadFuncBase*)obj),func(functype) {}
	ThreadWorker(const ThreadWorker& worker) {
		thiz = worker.thiz;
		func = worker.func;
	}
	ThreadWorker& operator=(const ThreadWorker& worker) {
		if (this != &worker) {
			thiz = worker.thiz;
			func = worker.func;
		}
		return *this;
	}
	int operator()() {
		if (isValid()) {
			if (thiz) {
				return (thiz->*func)();
			}
		}
		return -1;
	}
	bool isValid() const {
		return thiz!=NULL && func!=NULL;
	}
	ThreadFuncBase* thiz;  //ThreadFuncBase����ָ��
	FUNCTYPE func;   //ThreadFuncBase��Ա����ָ��
};


//worker�߳���
class CMyThread  
{//�߳���
public:
	CMyThread() {
		m_hThread = NULL;
		m_bStatus = false;
	}
	~CMyThread() {
		Stop();
	}
	bool Start() {//true��ʾ�ɹ�
		m_bStatus = true;
		m_hThread = (HANDLE)_beginthread(&CMyThread::thread_Entry, 0, this);  //�����߳�
		if (m_hThread == INVALID_HANDLE_VALUE) m_bStatus = false;
		return m_bStatus;
	}
	bool Stop() {
		if (m_bStatus == false) return true;
		m_bStatus = false;//��ֹthreadworker����
		DWORD ret = WaitForSingleObject(m_hThread, 1000) == WAIT_OBJECT_0;  //����ֵWAIT_OBJECT_0��ʾ�߳��ѽ���
		if (ret == WAIT_TIMEOUT) {
			TerminateThread(m_hThread, -1);  //ǿ�ƽ����߳�
		}
		UpdateWorker();
		return ret;
	}
	bool isValid(){//����true��ʾ��Ч false��ʾ�߳��쳣
		if (m_hThread == INVALID_HANDLE_VALUE || m_hThread == NULL) {
			return false;
		}
		return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;  //��ʾ�߳���ָ��ʱ��ûִ���꣬���߳�����ִ��
	}
	void UpdateWorker(const ::ThreadWorker& worker=::ThreadWorker()) {
		if (m_worker.load() != NULL && m_worker.load()!= &worker) {
			::ThreadWorker* pWorker = m_worker.load();
			TRACE("delete pWorker = %08X m_worker=%08X\r\n", pWorker, m_worker.load());
			m_worker.store(NULL);
			delete pWorker;
		}
		if (m_worker.load() == &worker) return;
		if (!worker.isValid()) {
			m_worker.store(NULL);
			return;
		}
		::ThreadWorker* pworker = new ::ThreadWorker(worker);
		TRACE("new pWorker = %08X m_worker=%08X\r\n", pworker, m_worker.load());
		m_worker.store(new ::ThreadWorker(worker));
	}
	bool IsIdle() {//�Ƿ���� true��ʾ���� false��ʾ���ڹ���
		if (m_worker.load() == NULL) {
			return true;  //��ʱΪ�գ�����״̬
		}
		return !m_worker.load()->isValid();  //m_worker����Ҫ����
	}
protected:
	//virtual int each_step();//���麯����Ĭ��Ҫ���û�ʵ��  ����ֵ0��ʾ����
private:
	void threadWorker() {//��������
		while (m_bStatus) {
			if (m_worker.load() == NULL) {
				Sleep(1);
				continue;
			}
			::ThreadWorker worker = *m_worker.load();
			if (worker.isValid()) {
				if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
					int ret = worker();   //()���� �൱�ڵ���worker��func
					if (ret != 0) {//����0���;�����־ ����0����������
						std::string str;
						str.resize(64);
						sprintf((char*)str.c_str(), "thread fuond warning code: %d\n", ret);
						OutputDebugStringA(str.c_str());
					}
					if (ret < 0) {//retС��0ʱ����ֹ�߳�ѭ��
						//m_worker.store(worker);
						::ThreadWorker* pWorker = m_worker.load();
						m_worker.store(NULL);
						delete pWorker;
					}
				}
			}
			else {
				Sleep(1);
			}
			
		}
	}
	static void thread_Entry(void* arg) {
		CMyThread* thiz = (CMyThread*)arg;
		thiz->threadWorker();
		_endthread();
	}
private:
	HANDLE m_hThread;
	bool m_bStatus;//false��ʾ�ر� true��ʾ�߳�����
	std::atomic<::ThreadWorker*> m_worker;
};


class MyThreadPoor {
public:
	MyThreadPoor(){}
	~MyThreadPoor() {
		Stop();
		for (int i = 0; i < m_threads.size(); i++) {
			delete m_threads[i];
			m_threads[i] = NULL;
		}
		m_threads.clear();
	}
	MyThreadPoor(size_t size) {  //�����̳߳ش�С
		m_threads.resize(size);
		for (size_t i = 0; i < size; i++) {
			m_threads[i] = new CMyThread();
		}
	}
	bool Invoke() {
		bool ret = true;
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->Start() == false) {//�߳�����ʧ��
				ret = false;
				break;
			}
		}
		if (ret = false) {//�߳�����ʧ�ܣ�ȫ���ر� �̳߳��У�����߳�û��ȫ�������ɹ�����ô����Ϊ�̳߳���ʧ�ܵ�
			for (size_t i = 0; i < m_threads.size(); i++) {
				m_threads[i]->Stop();
			}
		}
		return ret;
	}
	void Stop() {
		for (size_t i = 0; i < m_threads.size(); i++) {
			m_threads[i]->Stop();
		}
	}
	int dispatchWorker(const ThreadWorker& worker) {
		int index = -1;
		m_mutex.lock();
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->IsIdle()) {
				m_threads[i]->UpdateWorker(worker);
				index = i;
				break;
			}
		}
		m_mutex.unlock();
		return index;//������-1����֤���̳߳��������̶߳���æµ������ʧ��  ����ֵ���ʾ��������߳��±�
	}
	bool CheckThreadValid(size_t index) {  //����߳��Ƿ���Ч
		if (index < m_threads.size()) {
			return m_threads[index]->isValid();
		}
		return false;
	}

private:
	std::mutex m_mutex;
	std::vector<CMyThread*> m_threads;
};
