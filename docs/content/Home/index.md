# HakkaJson: Cutting Runtime Memory Costs

## Use cases
- Memory constrained environments where memory usage is a concern.
- Ideal for scenarios where small objects or string dominate the JSON data.
    - Objects are small. String values are short. Arrays are small.
    - The same values are repeated throughout.
- Suitable for unique integer or double values not exceeding 2^30 (1,073,741,824), including negative values.
    - For example: `[1, 1, 2, 3, 3, 3, 6]` holds **4** unique values, please do not exceed 2^30 unique values.
        - A future update will include an option to disable this limit.
- Compliant with the [RFC 8259](https://datatracker.ietf.org/doc/html/rfc8259) JSON standard.
- Fully tested on x86_64 and arm64 architectures. Other architectures are conceptually supported but not tested.
    - Yes, 32-bits are conceptually supported but not tested yet.

## License

Copyright (c) 2025, scc@cycraft

This software is dual-licensed under the Boost Software License 1.0 and the BSD 3-Clause License.
