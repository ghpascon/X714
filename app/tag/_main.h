#pragma once
#include <string.h>
#include <stdint.h>

// ==================== TagRecord ====================
// Armazenamento fisico de uma tag RFID.
// Usa char arrays fixos para eliminar fragmentacao de heap.
struct TagRecord
{
	char epc[25];	   // hex EPC null-terminated  (24 chars)
	char tid[25];	   // hex TID null-terminated  (24 chars) — chave unica
	int8_t ant_number; // porta de antena (1-4)
	int8_t rssi;	   // valor de RSSI
};

// ==================== TagStore ====================
// Armazenamento com capacidade fixa e lookup O(1) medio.
// Hash aberta (linear probing) com TID como chave unica.
// Zero alocacao de heap — toda memoria e estatica.
class TagStore
{
public:
	static constexpr int CAPACITY = 300;	  // max tags simultaneas
	static constexpr int HASH_CAP = 512;	  // deve ser potencia de 2, > CAPACITY
	static constexpr int16_t SLOT_EMPTY = -1; // sentinela: slot vazio

	TagStore() { _clear_tables(); }

	// Insere tag com chave no TID.
	// Retorna true se TID e novo e o registro foi salvo.
	// Retorna false se TID ja existe (duplicata) ou store esta cheio.
	bool upsert(const char *epc, const char *tid, int ant, int rssi)
	{
		if (!tid || tid[0] == '\0')
			return false;
		if (_count >= CAPACITY)
			return false;

		uint16_t idx = _hash(tid);
		for (uint16_t probe = 0; probe < HASH_CAP; probe++, idx = (idx + 1) & (HASH_CAP - 1))
		{
			const int16_t slot = _htid[idx];
			if (slot == SLOT_EMPTY)
			{
				TagRecord &r = _pool[_count];
				_strcpy24(r.epc, epc);
				_strcpy24(r.tid, tid);
				r.ant_number = (int8_t)ant;
				r.rssi = (int8_t)rssi;
				_htid[idx] = (int16_t)_count;
				_order[_count] = (int16_t)_count;
				_count++;
				return true;
			}
			if (strcmp(_pool[slot].tid, tid) == 0)
				return false; // TID duplicado
		}
		return false; // hash cheia (nao deve ocorrer com dimensionamento correto)
	}

	bool containsTid(const char *tid) const
	{
		if (!tid || tid[0] == '\0')
			return false;
		uint16_t idx = _hash(tid);
		for (uint16_t probe = 0; probe < HASH_CAP; probe++, idx = (idx + 1) & (HASH_CAP - 1))
		{
			const int16_t slot = _htid[idx];
			if (slot == SLOT_EMPTY)
				return false;
			if (strcmp(_pool[slot].tid, tid) == 0)
				return true;
		}
		return false;
	}

	int size() const { return _count; }
	bool isFull() const { return _count >= CAPACITY; }

	// Acesso por indice de insercao (0 = primeiro inserido).
	const TagRecord *get(int i) const
	{
		if (i < 0 || i >= _count)
			return nullptr;
		return &_pool[_order[i]];
	}

	void clear()
	{
		_count = 0;
		_clear_tables();
	}

private:
	TagRecord _pool[CAPACITY];
	int16_t _htid[HASH_CAP];  // TID hash → indice no pool (-1 = vazio)
	int16_t _order[CAPACITY]; // ordem de insercao → indice no pool
	int _count = 0;

	void _clear_tables()
	{
		// 0xFF repetido: int16_t 0xFFFF == -1 == SLOT_EMPTY
		memset(_htid, 0xFF, sizeof(_htid));
	}

	static void _strcpy24(char *dst, const char *src)
	{
		if (src)
			strncpy(dst, src, 24);
		else
			dst[0] = '\0';
		dst[24] = '\0';
	}

	// Hash FNV-1a 32-bit mascarada ao tamanho da tabela
	static uint16_t _hash(const char *s)
	{
		uint32_t h = 2166136261u;
		while (*s)
		{
			h ^= (uint8_t)*s++;
			h *= 16777619u;
		}
		return (uint16_t)(h & (HASH_CAP - 1));
	}
};