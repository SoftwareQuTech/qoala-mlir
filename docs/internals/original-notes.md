# Original notes

!!! note
    This page is the original `docs/README.md` content, kept here verbatim as a starting point until its contents have been redistributed across the structured docs. Expect it to be removed in a later cleanup pass.

## Implementation-specific documentation

The design rationale of the Qoala compiler — its IR architecture, lowering pipeline, optimization passes, and
analyses — is documented in the [compiler paper](<PAPER_URL>). The notes in this directory complement that
material with implementation-specific intricacies that the paper does not cover at the source-code level.

Please note that most of the details explained in these documents refer to *implementation-specific details*
or explain *the algorithms implemented to realize what the paper describes at the design level*.

## Index

- ["Functionization" algorithm](functionize.md)
