#pragma once

#include <QString>
#include <QStringView>

namespace AutostartEntry {

enum class State {
    Disabled,
    Enabled,
    Stale,
    Invalid,
};

[[nodiscard]] QString quoteExecArgument(QString argument);
[[nodiscard]] QString expectedExec(const QString &executable);
[[nodiscard]] QString build(const QString &executable);
[[nodiscard]] State inspect(QStringView contents, const QString &executable);

}
