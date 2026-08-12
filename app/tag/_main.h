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
		return _find_pool_index_by_tid(tid) >= 0;
	}

	const TagRecord *findByTid(const char *tid) const
	{
		const int idx = _find_pool_index_by_tid(tid);
		if (idx < 0)
			return nullptr;
		return &_pool[idx];
	}

	bool removeByTid(const char *tid)
	{
		const int idx = _find_pool_index_by_tid(tid);
		if (idx < 0)
			return false;
		return _remove_by_pool_index((int16_t)idx);
	}

	bool removeOldest()
	{
		if (_count <= 0)
			return false;
		return _remove_by_pool_index(_order[0]);
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

	void _rebuild_hash()
	{
		_clear_tables();
		for (int i = 0; i < _count; i++)
		{
			uint16_t idx = _hash(_pool[i].tid);
			for (uint16_t probe = 0; probe < HASH_CAP; probe++, idx = (idx + 1) & (HASH_CAP - 1))
			{
				if (_htid[idx] == SLOT_EMPTY)
				{
					_htid[idx] = (int16_t)i;
					break;
				}
			}
		}
	}

	int _find_pool_index_by_tid(const char *tid) const
	{
		if (!tid || tid[0] == '\0')
			return -1;

		uint16_t idx = _hash(tid);
		for (uint16_t probe = 0; probe < HASH_CAP; probe++, idx = (idx + 1) & (HASH_CAP - 1))
		{
			const int16_t slot = _htid[idx];
			if (slot == SLOT_EMPTY)
				return -1;
			if (strcmp(_pool[slot].tid, tid) == 0)
				return slot;
		}
		return -1;
	}

	bool _remove_by_pool_index(int16_t pool_index)
	{
		if (pool_index < 0 || pool_index >= _count)
			return false;

		int order_pos = -1;
		for (int i = 0; i < _count; i++)
		{
			if (_order[i] == pool_index)
			{
				order_pos = i;
				break;
			}
		}
		if (order_pos < 0)
			return false;

		for (int i = order_pos; i < _count - 1; i++)
			_order[i] = _order[i + 1];

		const int16_t last_index = (int16_t)(_count - 1);
		if (pool_index != last_index)
		{
			_pool[pool_index] = _pool[last_index];
			for (int i = 0; i < _count - 1; i++)
			{
				if (_order[i] == last_index)
				{
					_order[i] = pool_index;
					break;
				}
			}
		}

		_count--;
		_rebuild_hash();
		return true;
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