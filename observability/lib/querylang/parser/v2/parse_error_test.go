package parserv2_test

import (
	"errors"
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	parserv2 "github.com/yandex/perforator/observability/lib/querylang/parser/v2"
)

func TestNewParseError(t *testing.T) {
	t.Run("return nil if no errors", func(t *testing.T) {
		err := parserv2.NewParseError(make([]error, 0), make([]error, 0))

		require.NoError(t, err)
	})

	t.Run("return syntax error", func(t *testing.T) {
		syntaxErr := fmt.Errorf("cannot parse")
		err := parserv2.NewParseError([]error{syntaxErr}, make([]error, 0))

		require.Error(t, err)
		assert.EqualError(t, err, "syntax errors: cannot parse")
	})

	t.Run("return semantic error", func(t *testing.T) {
		semanticErr := fmt.Errorf("cannot parse")

		err := parserv2.NewParseError(make([]error, 0), []error{semanticErr})

		require.Error(t, err)
		assert.EqualError(t, err, "semantic errors: cannot parse")
	})

	t.Run("return syntax error even if there is syntax error", func(t *testing.T) {
		syntaxErr := fmt.Errorf("cannot parse")
		semanticErr := fmt.Errorf("cannot parse")

		err := parserv2.NewParseError([]error{syntaxErr}, []error{semanticErr})

		require.Error(t, err)
		assert.EqualError(t, err, "syntax errors: cannot parse")
	})

	t.Run("return correct ParseError type for syntax error", func(t *testing.T) {
		err := parserv2.NewParseError([]error{fmt.Errorf("cannot parse")}, make([]error, 0))

		var parseError *parserv2.ParseError
		correctType := errors.As(err, &parseError)

		require.True(t, correctType)
		require.Error(t, parseError)
	})

	t.Run("return correct ParseError type for semantic error", func(t *testing.T) {
		err := parserv2.NewParseError(make([]error, 0), []error{fmt.Errorf("cannot parse")})

		var parseError *parserv2.ParseError
		correctType := errors.As(err, &parseError)

		require.True(t, correctType)
		require.Error(t, parseError)
	})
}
