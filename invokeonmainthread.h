#ifndef INVOKEONMAINTHREAD_H
#define INVOKEONMAINTHREAD_H

#include <QCoreApplication>
#include <QMetaObject>
#include <optional>
#include <type_traits>

/**
 * @brief Runs the given callable \p f on the main Qt thread, then returns immediately (non-blocking).
 *
 * Example:
 *   invokeOnMainThreadAsync([]() {
 *       qDebug() << "I am now running in the main thread!";
 *   });
 */
template <typename F>
void invokeOnMainThreadAsync(F&& f)
{
    QMetaObject::invokeMethod(
        QCoreApplication::instance(),
        std::forward<F>(f),
        Qt::QueuedConnection
        );
}

/**
 * @brief Runs the given callable \p f on the main Qt thread, waits for it to complete, and returns its result (blocking).
 *
 * If \p f returns void, this function also returns void. Otherwise, it returns whatever \p f returns.
 *
 * Example:
 *   int result = invokeOnMainThreadBlocking([]() -> int {
 *       qDebug() << "I am in the main thread, returning 42";
 *       return 42;
 *   });
 */
template <typename F>
auto invokeOnMainThreadBlocking(F&& f)
{
    using ReturnType = std::invoke_result_t<F>;

    // If f returns non-void, we store the result in an optional
    std::optional<ReturnType> result;

    // We need a lambda that sets `result` if ReturnType != void
    auto wrapper = [&]() {
        if constexpr (std::is_void_v<ReturnType>) {
            std::forward<F>(f)();
        } else {
            result = std::forward<F>(f)();
        }
    };

    // Invoke on main thread, blocking until it finishes
    QMetaObject::invokeMethod(
        QCoreApplication::instance(),
        std::move(wrapper),
        Qt::BlockingQueuedConnection
        );

    if constexpr (std::is_void_v<ReturnType>) {
        // For void returns, do nothing
    } else {
        // For non-void returns, unwrap and return
        return *result;
    }
}

#endif // INVOKEONMAINTHREAD_H
