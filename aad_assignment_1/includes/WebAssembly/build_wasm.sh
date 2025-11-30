
set -e
cd "$(dirname "${BASH_SOURCE[0]}")"

if ! command -v emcc &> /dev/null; then
    echo "Error: emcc not found. Install with: sudo apt install emscripten"
    exit 1
fi

EXPORTS="-s MODULARIZE=1 -s EXPORT_NAME='createMinerModule'"
EXPORTS="$EXPORTS -s EXPORTED_FUNCTIONS=\"['_miner_init','_miner_start','_miner_stop','_miner_is_running','_miner_mine_batch','_miner_get_hashes_low','_miner_get_hashes_high','_miner_get_coins_found','_miner_get_coin_buffer_count','_miner_get_coin_buffer','_miner_clear_coin_buffer','_malloc','_free']\""
EXPORTS="$EXPORTS -s EXPORTED_RUNTIME_METHODS=\"['HEAPU32']\""
EXPORTS="$EXPORTS -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=16777216"

case "${1:-release}" in
    "simd")
        echo "Building WebAssembly SIMD miner..."
        eval emcc -O3 -msimd128 -I.. $EXPORTS aad_sha1_wasm_miner.c -o miner_simd.js
        echo "Built: miner_simd.js, miner_simd.wasm"
        ;;
    *)
        echo "Building WebAssembly miner..."
        eval emcc -O3 -flto -I.. $EXPORTS aad_sha1_wasm_miner.c -o miner.js
        echo "Built: miner.js, miner.wasm"
        ;;
esac

echo "Run: python3 -m http.server 8080"
echo "Open: http://localhost:8080/index.html"
