//
//  decompress.hpp
//  toybox
//
//  Created by Fredrik on 2024-05-17.
//

#include "decompress.hpp"
#include "utility.hpp"

namespace toystd {
    
    class reader_c : public nocopy_c {
    public:
        reader_c(const uint8_t *source, size_t length) :
        _source(source),
        _length(length),
        _pos(0),
        _byte(0),
        _bits_rem(0)
        {}
        
        short read_bit() {
            if (_bits_rem <= 0) {
                assert(_pos < _length);
                _byte = _source[_pos++];
                _bits_rem = 8;
            }
            _bits_rem--;
            return (_byte >> (7 - _bits_rem)) & 1;
        }
        
        short read_byte() {
            if (_bits_rem < 8) {
                assert(_pos < _length);
                _byte = _source[_pos++];
                _bits_rem = 0;
            }
            return _byte;
        }
        
        short read_bits(int numBits) {
            assert(numBits >= 0 && numBits <= 15);
            int result = 0;
            for (int i = 0; i < numBits; i++) {
                int bit = read_bit();
                result |= bit << i;
            }
            return result;
        }
        
        long read_bytes(int numBytes) {
            long result = 0;
            for (int i = 0; i < numBytes; i++) {
                int byte = read_byte();
                result |= byte << (i * 8);
            }
            return result;
        }
        
    private:
        const uint8_t *_source;
        size_t _length;
        size_t _pos;
        short _byte;
        short _bits_rem;
    };
    
    class writer_c : public nocopy_c {
    public:
        writer_c(uint8_t *dest, size_t length) :
        _dest(dest),
        _length(length),
        _pos(0)
        {}
        
        void write_byte(uint8_t byte) {
            assert(_pos < _length);
            _dest[_pos++] = byte;
        }
        
        void copy_bytes(long distance, short length) {
            uint8_t *from = &_dest[_pos - distance];
            assert(from >= _dest);
            while (length-- > 0) {
                write_byte(*from++);
            }
        }
        
        size_t size() const { return _pos; }
        
    private:
        uint8_t *_dest;
        size_t _length;
        size_t _pos;
    };
    
    static const short* make_fixed_length_bl() {
        static short result[289];
        int i = 0;
        for (; i < 144; i++) result[i] = 8;
        for (; i < 256; i++) result[i] = 9;
        for (; i < 280; i++) result[i] = 7;
        for (; i < 288; i++) result[i] = 8;
        result[i] = -1;
        return result;
    }
    static const short* make_fixed_distance_bl() {
        static short result[33];
        for (int i = 0; i < 32; i++) {
            result[i] = 5;
        }
        result[32] = -1;
        return result;
    }
    static short list_length(const short *list) {
        short length = 0;
        while (*list++ != -1) {
            length++;
        }
        return length;
    }
    
    class huffman_tree_c : public nocopy_c {
    public:
        huffman_tree_c(const short *code_lengths) :
        _free_node(new node_c[list_length(code_lengths) * 2 - 1]),
        _root(_free_node++)
        {
            short max_length = 0;
            for (short i = 0, len; (len = code_lengths[i]) != -1; i++) {
                if (len > max_length) {
                    max_length = len;
                }
            }
            
            short bl_count[max_length + 1];
            memset(bl_count, 0, sizeof(short) * (max_length + 1));
            for (short i = 0, len; (len = code_lengths[i]) != -1; i++) {
                if (len != 0) {
                    bl_count[len]++;
                }
            }
            
            short next_code[max_length + 1];
            next_code[0] = next_code[1] = 0;
            for (short bits = 2; bits <= max_length; bits++) {
                next_code[bits] = (next_code[bits - 1] + bl_count[bits - 1]) << 1;
            }
            
            for (short c = 0, bitlen; (bitlen = code_lengths[c]) != -1; c++) {
                if (bitlen != 0) {
                    insert_symbol(next_code[bitlen], bitlen, c);
                    next_code[bitlen]++;
                }
            }
        }
        ~huffman_tree_c() { delete _root; }
        
        short decode_symbol(reader_c &in) const {
            node_c *node = _root;
            while (node->left || node->right) {
                short b = in.read_bit();
                node = b ? node->right : node->left;
            }
            return node->symbol;
        }
        
        void insert_symbol(short code, short len, short symbol) {
            node_c *node = _root;
            for (int i = len - 1; i >= 0; i--) {
                node_c *next_node = nullptr;
                short b = code & (1 << i);
                if (b) {
                    next_node = node->right;
                    if (next_node == nullptr) {
                        node->right = next_node = _free_node++;
                    }
                } else {
                    next_node = node->left;
                    if (next_node == nullptr) {
                        node->left = next_node = _free_node++;
                    }
                }
                node = next_node;
            }
            node->symbol = symbol;
        }
        
    private:
        class node_c : public nocopy_c {
        public:
            node_c() : symbol(0), left(nullptr), right(nullptr) {}
            short symbol;
            node_c *left;
            node_c *right;
        };
        node_c *_free_node;
        node_c *_root;
    };
    
    class decompressor_c : public nocopy_c {
    public:
        decompressor_c(uint8_t *dest, size_t dest_length, const uint8_t *source, size_t source_length) :
        _reader(source, source_length),
        _writer(dest, dest_length)
        {}
        
        size_t decompress() {
            bool is_final;
            do {
                is_final = _reader.read_bits(1) != 0;
                const int type = _reader.read_bits(2);
                switch (type) {
                    case 0:
                        do_literal_block();
                        break;
                    case 1:
                        do_huffman_block(fixed_length_tree(), fixed_distance_tree());
                        break;
                    case 2: {
                        huffman_tree_c *length_tree = nullptr;
                        huffman_tree_c *dist_tree = nullptr;
                        decode_huffman_trees(length_tree, dist_tree);
                        do_huffman_block(*length_tree, *dist_tree);
                        delete length_tree;
                        delete dist_tree;
                        break;
                    }
                    default:
                        assert(0);
                }
            } while (!is_final);
            return _writer.size();
        }
        
    private:
        static const huffman_tree_c &fixed_length_tree() {
            static huffman_tree_c tree(make_fixed_length_bl());
            return tree;
        }
        static const huffman_tree_c &fixed_distance_tree() {
            static huffman_tree_c tree(make_fixed_distance_bl());
            return tree;
        }
        reader_c _reader;
        writer_c _writer;
        
        void do_literal_block() {
            long  len = _reader.read_bytes(2);
            long nlen = _reader.read_bytes(2);
            assert((len ^ 0xFFFF) == nlen);
            
            for (long i = 0; i < len; i++) {
                short b = _reader.read_bytes(1);
                _writer.write_byte(static_cast<uint8_t>(b));
            }
        }
        
        void do_huffman_block(const huffman_tree_c &length_tree, const huffman_tree_c &distance_tree) {
            while (true) {
                short sym = length_tree.decode_symbol(_reader);
                if (sym == 256) {
                    break;
                } else if (sym < 256) {
                    uint8_t b = static_cast<uint8_t>(sym);
                    _writer.write_byte(b);
                } else {
                    short length = decode_length(sym);
                    assert(length >= 3 && length <= 258);
                    short dist_sym = distance_tree.decode_symbol(_reader);
                    long distance = decode_distance(dist_sym);
                    assert(distance >= 1 && distance <= 32768);
                    _writer.copy_bytes(distance, length);
                }
            }
        }
        
        void decode_huffman_trees(huffman_tree_c *&length_tree, huffman_tree_c *&dist_tree) {
            static const short length_order[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
            const short length_code_count = _reader.read_bits(5) + 257;  // hlit + 257
            const short distance_code_count = _reader.read_bits(5) + 1;      // hdist + 1
            
            const short code_length_count = _reader.read_bits(4) + 4;   // hclen + 4
            short code_length_bl[20];  // This array is filled in a strange order
            memset(code_length_bl, 0, sizeof(short) * 20);
            for (int i = 0; i < code_length_count; i++) {
                code_length_bl[length_order[i]] = _reader.read_bits(3);
            }
            code_length_bl[19] = -1;
            huffman_tree_c code_length_tree(code_length_bl);
            
            // Read the main code lengths and handle runs
            short code_lengths[static_cast<unsigned int>(length_code_count + distance_code_count)];
            short code_length_pos = 0;
            while (code_length_pos < (length_code_count + distance_code_count)) {
                const short sym = code_length_tree.decode_symbol(_reader);
                if (0 <= sym && sym <= 15) {
                    code_lengths[code_length_pos++] = sym;
                } else {
                    short repeat_sym = 0;
                    short repeat_length = 0;
                    if (sym == 16) {
                        assert(code_length_pos > 0);
                        repeat_length = _reader.read_bits(2) + 3;
                        repeat_sym = code_lengths[code_length_pos - 1];
                    } else if (sym == 17) {
                        repeat_length = _reader.read_bits(3) + 3;
                    } else if (sym == 18) {
                        repeat_length = _reader.read_bits(7) + 11;
                    } else {
                        assert(0);
                    }
                    for (short i = 0; i < repeat_length; i++) {
                        code_lengths[code_length_pos++] = repeat_sym;
                    }
                }
            }
            assert(code_length_pos <= static_cast<unsigned int>(length_code_count + distance_code_count));
            
            short length_code_bl[length_code_count + 1];
            memcpy(length_code_bl, code_lengths, sizeof(short) * length_code_count);
            length_code_bl[length_code_count] = -1;
            assert(length_code_bl[256] != 0);
            length_tree = new huffman_tree_c(length_code_bl);
            
            short distance_code_bl[33];
            memcpy(distance_code_bl, code_lengths + length_code_count, sizeof(short) * distance_code_count);
            memset(distance_code_bl + distance_code_count, 0, sizeof(short) * (33 - distance_code_count));
            distance_code_bl[distance_code_count] = -1;
            size_t one_count = 0;
            size_t gt_one_count = 0;
            for (int i = 0, x; (void)(x = distance_code_bl[i]), i < code_length_count; i++) {
                if (x == 1) {
                    one_count++;
                } else if (x > 1) {
                    gt_one_count++;
                }
            }
            
            if (one_count == 1 && gt_one_count == 0) {
                for (int i = code_length_count; i < 32; i++) {
                    distance_code_bl[i] = 0;
                }
                distance_code_bl[31] = 1;
                distance_code_bl[32] = -1;
            }

            dist_tree = new huffman_tree_c(distance_code_bl);
        }
        
        short decode_length(short sym) {
            static const short extra_bits[29] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3,
                3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };
            static const short base[29] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43,
                51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
            sym -= 257;
            assert(sym >= 0 && sym <= 28);
            return _reader.read_bits(extra_bits[sym]) + base[sym];
        }
        
        long decode_distance(short sym) {
            static const short extra_bits[30] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7,
                8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };
            static const short base[30] = { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257,
                385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385,
                24577 };
            assert(sym >= 0 && sym <= 29);
            return _reader.read_bits(extra_bits[sym]) + base[sym];
        }
    };
    
    size_t decompress_deflate(uint8_t *dest, size_t dest_length, const uint8_t *source, size_t source_length) {
        decompressor_c dc(dest, dest_length, source, source_length);
        return dc.decompress();
    }
    
}
