#pragma once

#include <ext/shared_ptr_helper.h>

#include <Storages/StorageSet.h>
#include <Parsers/ASTTablesInSelectQuery.h>


namespace DB
{

class Join;
using JoinPtr = std::shared_ptr<Join>;


/** Allows you save the state for later use on the right side of the JOIN.
  * When inserted into a table, the data will be inserted into the state,
  *  and also written to the backup file, to restore after the restart.
  * Reading from the table is not possible directly - only specifying on the right side of JOIN is possible.
  *
  * When using, JOIN must be of the appropriate type (ANY|ALL LEFT|INNER ...).
  */
class StorageJoin : public ext::shared_ptr_helper<StorageJoin>, public StorageSetOrJoinBase
{
public:
    String getName() const override { return "Join"; }

    void truncate(const ASTPtr &, const Context &) override;

    /// Access the innards.
    JoinPtr & getJoin() { return join; }

    /// Verify that the data structure is suitable for implementing this type of JOIN.
    void assertCompatible(ASTTableJoin::Kind kind_, ASTTableJoin::Strictness strictness_) const;

    BlockInputStreams read(
        const Names & column_names,
        const SelectQueryInfo & query_info,
        const Context & context,
        QueryProcessingStage::Enum processed_stage,
        size_t max_block_size,
        unsigned num_streams) override;

private:
    Block sample_block;
    const Names key_names;
    bool use_nulls;
    SizeLimits limits;
    ASTTableJoin::Kind kind;                    /// LEFT | INNER ...
    ASTTableJoin::Strictness strictness;        /// ANY | ALL

    JoinPtr join;

    void insertBlock(const Block & block) override;
    size_t getSize() const override;

protected:
    StorageJoin(
        const String & path_,
        const String & name_,
        const Names & key_names_,
        bool use_nulls_,
        SizeLimits limits_,
        ASTTableJoin::Kind kind_, ASTTableJoin::Strictness strictness_,
        const ColumnsDescription & columns_,
        bool overwrite);
};

}
