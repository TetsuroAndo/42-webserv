#pragma once

#include <cstdlib>

/**
 * @brief 高品質なシード値を生成します。
 * @return 生成されたシード値（unsigned int）
 *
 * 複数のエントロピーソースを組み合わせてシードを生成します：
 * - システム時刻（秒単位）
 * - /dev/urandom（利用可能な場合）
 * - ビットミキシングによるエントロピー強化
 */
unsigned int generateSeed();
